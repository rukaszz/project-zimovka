#ifndef ZIMOVKA_SYSTEMS_ENEMY_ENEMYSPAWNPARAMS_HPP_
#define ZIMOVKA_SYSTEMS_ENEMY_ENEMYSPAWNPARAMS_HPP_

#include <cstdint>
#include <numbers>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/enemy/EnemyAttackPattern.hpp"

namespace zimovka{
/**
 * @brief EnemySystem::Spawn()へ渡す引数の構造体
 * 
 */
struct EnemySpawnParams{
    // ゲーム上の中心座標/速度
    Vec2 position{0, 0};
    Vec2 velocity{0, 0};
    // 描画用サイズ(描画時のみ左上座標で管理する)
    Vec2 render_size{32.0f, 32.0f};

    // 自機弾を受けるhurtbox
    Vec2 hurtbox_offset{};
    float hurtbox_radius = 13.0f;

    // Playerとの接触判定用半径
    Vec2 contact_offset{};
    float contact_radius = 10.0f;

    std::int32_t hp = 1;

    // 弾発射関係
    // 攻撃パターン
    EnemyAttackPattern attack_pattern      = EnemyAttackPattern::AimedSpread;
    // FixedSpread 用の固定発射角度
    float fixed_fire_angle_rad             = std::numbers::pi_v<float> * 0.5f;  // デフォルト: 真下
    std::uint32_t fire_bullet_count        = 5;                                 // 発射数
    float         fire_spread_rad          = std::numbers::pi_v<float> * 0.5f;  // 拡散角度(90°)
    float         fire_bullet_speed        = 180.0f;                            // 弾速
    std::uint32_t initial_fire_delay_ticks = 60;                                // スポーン時の発射ディレイ
    std::uint32_t fire_interval_ticks      = 120;                               // 発射間隔
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_ENEMY_ENEMYSPAWNPARAMS_HPP_
