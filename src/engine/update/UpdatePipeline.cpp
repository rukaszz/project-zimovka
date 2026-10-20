#include "zimovka/engine/update/UpdatePipeline.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/events/PlayerWeaponEvents.hpp"
#include "zimovka/events/EnemyHitEvents.hpp"
#include "zimovka/systems/bomb/PlayerBombEvents.hpp"
#include "zimovka/systems/boss/BossConfig.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"
#include "zimovka/stages/Stage1Data.hpp"

namespace zimovka{
/**
 * @brief 各システムの初期化
 * 冪等性を持つ
 * 
 * @param width 
 * @param height 
 */
void UpdatePipeline::Initialize(float width, float height){
    // 引数チェック(負の値とNaNのチェック)
    if(!std::isfinite(width) 
    || !std::isfinite(height)
    || width <= 0.0f || height <= 0.0f
    )
    {
        throw std::invalid_argument(
            "Game play world size must be positive. "
        );
    }
    world_width_  = width;
    world_height_ = height;
    // Player関係初期化
    player_system_.Initialize(width, height);
    player_weapon_system_.Reset();
    bomb_system_.Reset();
    // 敵初期化
    enemy_system_.Clear();
    enemy_spawner_ = EnemySpawner{Stage1::PROTOTYPE_EVENTS};
    // ステージ
    stage_phase_ = StagePhase::Normal;
    // ボス
    boss_system_.Reset();
    // BulletSystem初期化
    player_bullets_.Clear();
    enemy_bullets_.Clear();

    // ボス設定初期化
    // fight_tick はボスのEnteringフェーズの長さ(= 180 tick = 3秒)
    // 出現タイミング(tick 3600)はStage1::BOSS_SPAWN_TICKで管理する
    boss_config_.entering_duration_ticks = 180;
}

/**
 * @brief ゲーム実行開始用の初期化ラッパAPI
 * 画面サイズやシード値などを決定する
 * 
 * @param width 
 * @param height 
 * @param seed 
 */
void UpdatePipeline::StartRun(float width, float height, DeterministicRng::Seed seed){
    Initialize(width, height);
    gameplay_rng_.Reseed(seed);
    tick_index_ = 0;
}

/**
 * @brief ここで固定更新順序を実装する
 * 各サブシステムから伝搬されるイベントを受け取り上位へ伝搬させる
 * 
 * @param dt 
 * @param input 
 * @return * GameplayTickEvents 
 */
GameplayTickEvents UpdatePipeline::UpdateTick(float dt, const InputState& input){
    // イベント受け取り用
    GameplayTickEvents events{};
    // Simulation pipeline:
    // 1. Player movement
    // 2. Player weapon
    // 3. Enemy spawn
    // 4. Enemy movement
    // 5. Enemy fire
    // 6. Projectile movement
    // 7. Collision
    // 8. State resolution
    
    // NormalフェーズからBossフェーズへの遷移を監視
    // NOTE: 現状は強制移行，WAVE処理による移行も検討
    if(stage_phase_ == StagePhase::Normal){
        // ボス戦開始のTick
        if(tick_index_ >= Stage1::BOSS_SPAWN_TICK){
            StartBossPhase();
        } else {
            // ボス戦Tickまでは敵をスポーンさせる
            (void)enemy_spawner_.UpdateTick(tick_index_, enemy_system_);
        }
    }
    UpdatePlayer(dt, input);
    events.weapon = UpdateWeapons(input);
    UpdateEnemy(dt);
    UpdateBoss(dt);
    UpdateProjectiles(dt);
    bool raw_player_hit = false;
    // 被弾したかどうかのみを見る(raw_palyer_hitは参照で渡す)
    ResolveCollisions(raw_player_hit, events.enemy_hit, events.boss_hit, events.bullet_cancel);
    // ボム処理開始
    events.bomb = UpdateBomb(input, raw_player_hit);
    // 被弾確定時の後処理
    if(events.bomb.hit_applied){
        player_system_.Initialize(world_width_, world_height_);
        enemy_bullets_.Clear();
        player_bullets_.Clear();
        player_weapon_system_.Reset();
        events.player_hit = true;
    }
    
    // ボス撃破判定
    if(events.boss_hit.defeated_boss){
        enemy_bullets_.Clear();
        stage_phase_ = StagePhase::Cleared;
    }

    // 最後に伝搬したイベントを返す
    ++tick_index_;
    return events;
}

/**
 * @brief Playerの更新
 * 
 * @param dt 
 * @param input 
 */
void UpdatePipeline::UpdatePlayer(float dt, const InputState& input){
    player_system_.Update(dt, input);
}

/**
 * @brief 敵の更新
 * 移動・画面外除去はEnemySystem::Update，発射タイマーと弾生成はEnemySystem::UpdateFireへ委譲
 *
 * @param dt
 */
void UpdatePipeline::UpdateEnemy(float dt){
    enemy_system_.Update(dt, world_width_, world_height_);
    enemy_system_.UpdateFire(
        pattern_system_,
        enemy_bullets_,
        player_system_.GetPlayerPosition()
    );
}

/**
 * @brief PlayerWeaponSystemの更新
 * 
 * @param input 
 */
PlayerWeaponEvents UpdatePipeline::UpdateWeapons(const InputState& input){
    const PlayerWeaponEvents events = 
        player_weapon_system_.UpdateTick(
            input, 
            player_system_.GetPlayer(), 
            player_bullets_
        );
    // PlayerWeaponSystemから伝搬したイベントを返す
    return events;
}

/**
 * @brief 弾関係の更新
 * 
 * @param dt 
 */
void UpdatePipeline::UpdateProjectiles(float dt){
    // 自機弾の更新
    player_bullets_.Update(
        dt, 
        world_width_, 
        world_height_
    );
    // 敵弾の更新
    enemy_bullets_.Update(
        dt, 
        world_width_, 
        world_height_
    );
}

/**
 * @brief 当たり判定の検出処理
 * 被弾の解決はUpdateBombへ委譲するため，ここでは検出のみを行う
 *
 */
void UpdatePipeline::ResolveCollisions(
    bool& player_hit_out, 
    EnemyHitEvents& enemy_hit_out,
    BossHitEvents& boss_hit_out, 
    BulletCancelEvents& bullet_cancel_out
)
{
    // Collision判定回数の初期化
    collision_system_.InitializeStatsAtBeginTick();
    // 1. Player Bullets vs Enemy Bullets
    bullet_cancel_out = collision_system_.ResolvePlayerBulletsVsEnemyBullets(
        player_bullets_,
        enemy_bullets_
    );
    // 2. PlayerBullet vs Enemy
    enemy_hit_out = collision_system_.ResolvePlayerBulletsVsEnemies(
        player_bullets_,
        enemy_system_
    );
    // 3. PlayerBullet vs Boss
    boss_hit_out = collision_system_.ResolvePlayerBulletsVsBoss(
        player_bullets_,
        boss_system_
    );
    // 4. Player vs EnemyBullet(検出のみ，解決はUpdateBombで行う)
    const bool bullet_hit = collision_system_.CheckPlayerHitByBullets(
        player_system_.GetPlayer(),
        enemy_bullets_
    );
    // 5. Player vs Enemy
    const bool contact_hit_by_enemy = collision_system_.CheckPlayerHitByEnemies(
        player_system_.GetPlayer(), 
        enemy_system_
    );
    // 6. Player vs Boss
    const bool contact_hit_by_boss = collision_system_.CheckPlayerHitByBoss(
        player_system_.GetPlayer(), 
        boss_system_.GetBoss()
    );
    // 敵弾，敵との接触判定の結果で被弾判定を確定
    player_hit_out = bullet_hit || contact_hit_by_enemy || contact_hit_by_boss;
}

/**
 * @brief ボムシステムの更新
 * ResolveCollisionsで検出した被弾をボムでキャンセル or 確定させる
 *
 * @param input
 * @param player_hit ResolveCollisionsの生の検出結果
 * @return PlayerBombEvents
 */
PlayerBombEvents UpdatePipeline::UpdateBomb(const InputState& input, bool player_hit){
    return bomb_system_.UpdateTick(input, player_hit, enemy_bullets_, enemy_system_);
}

/**
 * @brief ボスシステムの更新
 * 
 * @param dt 
 */
void UpdatePipeline::UpdateBoss(float dt){
    if(boss_system_.IsNotInactive()){
        boss_system_.UpdateTick(
            dt, 
            player_system_.GetPlayerPosition(), 
            pattern_system_, 
            enemy_bullets_
        );
    }
}

/**
 * @brief ボス戦開始処理 
 * 
 */
void UpdatePipeline::StartBossPhase(){
    enemy_system_.Clear();
    enemy_bullets_.Clear();
    boss_system_.Start(boss_config_);
    stage_phase_ = StagePhase::Boss;
}

}   // namespace zimovka
