#include "zimovka/engine/update/UpdatePipeline.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/events/PlayerWeaponEvents.hpp"
#include "zimovka/events/EnemyHitEvents.hpp"
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
    // 敵初期化
    enemy_system_.Clear();
    // BulletSystem初期化
    player_bullets_.Clear();
    enemy_bullets_.Clear();
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
    // 2. Weapon and projectile spawning
    // 3. Projectile movement
    // 4. Collision detection
    // 5. Gameplay state resolution
    UpdatePlayer(dt, input);
    events.weapon = UpdateWeapons(input);
    UpdateEnemy(dt);
    UpdateProjectiles(dt);
    ResolveCollisions(events.player_hit, events.enemy_hit);

    // 最後に伝搬したイベントを返す
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
 * 
 * @param dt 
 */
void UpdatePipeline::UpdateEnemy(float dt){
    SpawnEnemyTest();
    enemy_system_.Update(dt, world_width_, world_height_);
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
 * @brief 当たり判定の処理
 * 現状はGameplayTickEventsのplayer_hitを返す
 * 
 * @return true 
 * @return false 
 */
void UpdatePipeline::ResolveCollisions(bool& player_hit_out, EnemyHitEvents& enemy_hit_out){
    // Collision判定回数の初期化
    collision_system_.InitializeStatsAtBeginTick();
    // 先に衝突処理
    // Player vs EnemyBullet
    const bool player_hit = collision_system_.CheckPlayerHitByBullets(
        player_system_.GetPlayer(), 
        enemy_bullets_
    );
    // PlayerBullet vs Enemy
    enemy_hit_out = collision_system_.ResolvePlayerBulletsVsEnemies(
        player_bullets_,
        enemy_system_
    );
    // 最後にプレイヤーの衝突の解決
    if(player_hit){
        // 衝突した場合はプレイヤーの位置を初期化(仮)
        player_system_.Initialize(
            world_width_, 
            world_height_
        );
        enemy_bullets_.Clear();
        player_bullets_.Clear();
        player_weapon_system_.Reset();
        player_hit_out = true;
    }
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
 * @brief 弾を瞬間的に大量生成する性能試験用関数
 *
 * 呼ばれると enemy_bullets_ を満杯まで生成する
 * NOTE: 将来的には性能試験用のビルドに移行する
 */
void UpdatePipeline::InitializeBulletStressTest(){
    for(std::size_t i = 0; i < enemy_bullets_.GetCapacity(); ++i){
        const float x = static_cast<float>(i % 40) * 24.0f + 12.0f;
        const float y = static_cast<float>(i / 40) * 16.0f;
        enemy_bullets_.Spawn(Vec2{x, y}, Vec2{0.0f, 60.0f}, 3.0f);
    }
}

/**
 * @brief 敵を1体出現させる関数
 * 
 * NOTE: 仮の実装
 */
void UpdatePipeline::SpawnEnemyTest(){
    const EnemySpawnParams params{
        .position = {world_width_*0.5f, world_height_*0.5f},
        .velocity = {10.0f, 10.0f}, 
        .render_size = {32.0f, 32.0f}, 
        .hurtbox_offset = {0.0f, 0.0f}, 
        .hurtbox_radius = 13.0f, 
        .contact_offset = {0.0f, 0.0f}, 
        .contact_radius = 10.0f, 
        .hp = 2
    };
    if(enemy_system_.CountActive() == 0){
        enemy_system_.Spawn(params);    
    }
}

}   // namespace zimovka
