#include "zimovka/engine/update/UpdatePipeline.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/events/PlayerWeaponEvents.hpp"
#include "zimovka/events/EnemyHitEvents.hpp"
#include "zimovka/systems/bomb/PlayerBombEvents.hpp"
#include "zimovka/systems/enemy/EnemySpawnParams.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"

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
    if( !std::isfinite(width) 
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
    // BulletSystem初期化
    player_bullets_.Clear();
    enemy_bullets_.Clear();
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
    // 3. Enemy movement
    // 4. Enemy fire
    // 5. Projectile movement
    // 6. Collision
    // 7. State resolution
    // ── 実装・性能試験用 ───────────────────────────────────────
    SpawnPhase1PrototypeEnemy();
    // ─────────────────────────────────────────
    UpdatePlayer(dt, input);
    events.weapon = UpdateWeapons(input);
    UpdateEnemy(dt);
    UpdateProjectiles(dt);
    bool raw_player_hit = false;
    // 被弾したかどうかのみを見る
    ResolveCollisions(raw_player_hit, events.enemy_hit);
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
void UpdatePipeline::ResolveCollisions(bool& player_hit_out, EnemyHitEvents& enemy_hit_out){
    // Collision判定回数の初期化
    collision_system_.InitializeStatsAtBeginTick();
    // Player vs EnemyBullet(検出のみ，解決はUpdateBombで行う)
    player_hit_out = collision_system_.CheckPlayerHitByBullets(
        player_system_.GetPlayer(),
        enemy_bullets_
    );
    // PlayerBullet vs Enemy
    enemy_hit_out = collision_system_.ResolvePlayerBulletsVsEnemies(
        player_bullets_,
        enemy_system_
    );
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
 * @brief 各システムの描画
 *
 * @param prim
 */
void UpdatePipeline::Render(PrimitiveRenderer& prim) const{
    // 敵更新
    enemy_system_.Render(prim);
    // 弾更新
    player_bullets_.Render(prim);
    enemy_bullets_.Render(prim);
    // プレイヤー更新
    player_system_.Render(prim);
}

/**
 * @brief 実際に乱数を消費して敵を生成する
 * 
 * NOTE: 仮の実装
 */
void UpdatePipeline::SpawnPhase1PrototypeEnemy(){
    // 約10秒周期で生成する
    if(tick_index_ % 625u != 0u){
        return;
    }

    // x座標はランダム
    const float spawn_x = static_cast<float>(
        gameplay_rng_.UniformU32(100u, 860u)    // 100〜860
    );
    // スピードでも乱数を消費
    const float speed = static_cast<float>(
        gameplay_rng_.UniformU32(30u, 70u)      // 30〜70
    );
    
    // 敵生成用引数を用意
    EnemySpawnParams params{};
    params.position       = {spawn_x, 80.0f};
    params.velocity       = {0.0f, speed};    // y軸のみ速度をつける
    params.render_size    = {32.0f, 32.0f};
    params.hurtbox_radius = 13.0f;
    params.contact_radius = 10.0f;
    params.hp             = 2;

    (void)enemy_system_.Spawn(params);
}

}   // namespace zimovka
