#ifndef ZIMOVKA_SYSTEMS_ENEMY_ENEMY_HPP_
#define ZIMOVKA_SYSTEMS_ENEMY_ENEMY_HPP_

#include <cstdint>
#include <numbers>

#include "zimovka/core/Circle.hpp"
#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/enemy/EnemyAttackPattern.hpp"

namespace zimovka{
/**
 * @brief 敵のデータ構造(コンポーネント)
 * 
 * 当たり判定(hurtbox)は円で管理し，大型の敵は円の組み合わせで表現する
 */
struct Enemy{
    // 活性/非活性で管理
    bool active = false;
    // ゲーム上の中心座標/速度
    Vec2 position{0, 0};
    Vec2 velocity{0, 0};
    // 描画用サイズ(描画時のみ左上座標で管理する)
    Vec2 render_size{32.0f, 32.0f};

    // 自機弾を受けるhurtbox
    Vec2 hurtbox_offset{};
    float hurtbox_radius = 13.0f;

    // Playerとの接触判定用円
    Vec2 contact_offset{};
    float contact_radius = 10.0f;

    std::int32_t hp = 1;

    // 弾発射関係
    // 攻撃パターン
    EnemyAttackPattern attack_pattern = EnemyAttackPattern::AimedSpread;
    // 非自機狙い拡散角度
    float fixed_fire_angle_rad = std::numbers::pi_v<float> * 0.5f;

    std::uint32_t fire_timer_ticks    = 0;                                         // カウントダウン
    std::uint32_t fire_interval_ticks = 120;                                        // 発射間隔
    std::uint32_t fire_bullet_count   = 5;                                          // 発射数
    float         fire_spread_rad     = std::numbers::pi_v<float> * 0.5f;          // 拡散角度
    float         fire_bullet_speed   = 180.0f;                                     // 弾速
    
    // 自機弾との当たり判定用円を返す
    Circle GetHurtboxCircle() const noexcept{
        return Circle{position + hurtbox_offset, hurtbox_radius};
    }
    // プレイヤーとの接触判定用円を返す
    Circle GetContactCircle() const noexcept{
        return Circle{position + contact_offset, contact_radius};
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_ENEMY_ENEMY_HPP_
