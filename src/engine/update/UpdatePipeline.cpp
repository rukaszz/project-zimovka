#include "zimovka/engine/update/UpdatePipeline.hpp"

#include <cassert>
#include <cstddef>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/events/PlayerWeaponEvents.hpp"
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
    // 引数チェック
    if(width <= 0.0f || height <= 0.0f){
        throw std::invalid_argument(
            "Game play world size must be positive. "
        );
    }
    world_width_  = width;
    world_height_ = height;
    // Player関係初期化
    player_system_.Initialize(width, height);
    player_weapon_system_.Reset();
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
 * @return * GamePlayTickEvents 
 */
GamePlayTickEvents UpdatePipeline::UpdateTick(float dt, const InputState& input){
    // イベント受け取り用
    GamePlayTickEvents events{};
    // Simulation pipeline:
    // 1. Player movement
    // 2. Weapon and projectile spawning
    // 3. Projectile movement
    // 4. Collision detection
    // 5. Gameplay state resolution
    UpdatePlayer(dt, input);
    events.weapon = UpdateWeapons(input);
    UpdateProjectiles(dt);
    events.player_hit = ResolveCollisions();

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
 * 現状はGamePlayTickEventsのplayer_hitを返す
 * 
 * @return true 
 * @return false 
 */
bool UpdatePipeline::ResolveCollisions(){
    // Player vs Bullet
    if(collision_system_.CheckPlayerHitByBullets(
        player_system_.GetPlayer(), 
        enemy_bullets_
    ))
    {
        // 衝突した場合はプレイヤーの位置を初期化(仮)
        player_system_.Initialize(
            world_width_, 
            world_height_
        );
        enemy_bullets_.Clear();
        player_bullets_.Clear();
        player_weapon_system_.Reset();
        return true;
    }
    return false;
}

/**
 * @brief 各システムの描画
 *
 * @param prim
 */
void UpdatePipeline::Render(PrimitiveRenderer& prim) const{
    player_bullets_.Render(prim);
    enemy_bullets_.Render(prim);
    player_system_.Render(prim);
}

/**
 * @brief 弾を瞬間的に大量生成する性能試験用関数
 *
 * 呼ばれると enemy_bullets_ を満杯まで生成する
 * NOTE: 将来的には EnemySystem に移行する
 */
void UpdatePipeline::InitializeBulletStressTest(){
    for(std::size_t i = 0; i < enemy_bullets_.GetCapacity(); ++i){
        const float x = static_cast<float>(i % 40) * 24.0f + 12.0f;
        const float y = static_cast<float>(i / 40) * 16.0f;
        enemy_bullets_.Spawn(Vec2{x, y}, Vec2{0.0f, 60.0f}, 3.0f);
    }
}

}   // namespace zimovka
