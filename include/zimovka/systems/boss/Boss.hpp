#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSS_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSS_HPP_

#include <cstdint>
#include <string>

#include "zimovka/core/Circle.hpp"
#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/boss/BossStatus.hpp"

namespace zimovka{
/**
 * @brief Boss関係のデータを管理する構造体
 *
 */
struct Boss{
    // 名前
    std::string name = "";
    // ボスの状態
    BossStatus status = BossStatus::Inactive;

    Vec2 position{};                        // 中心座標
    Vec2 velocity{};                        // 速度
    Vec2 render_size{};                     // 描画サイズ

    float hurtbox_radius = 0.0f;            // 当たり判定半径
    float contact_radius = 0.0f;            // 自機との衝突判定半径

    std::int32_t hp      = 0;               // 現体力
    std::int32_t max_hp  = 0;               // 体力最大値

    std::uint64_t entering_ticks_remaining  = 0; // 登場フェーズのカウントダウン
    std::uint64_t defeating_ticks_remaining = 0; // 敗北フェーズのカウントダウン

    std::uint32_t aimed_fire_timer         = 0;  // 自機狙い発射カウントダウン
    std::uint32_t aimed_fire_interval      = 0;  // 自機狙い発射リセット間隔
    std::uint32_t circle_fire_timer        = 0;  // 全方向発射カウントダウン
    std::uint32_t circle_fire_interval     = 0;  // 全方向発射リセット間隔

    // CollisionSystemから参照するための当たり判定円
    Circle GetHurtboxCircle() const noexcept{
        return Circle{position, hurtbox_radius};
    }
    Circle GetContactCircle() const noexcept{
        return Circle{position, contact_radius};
    }
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSS_HPP_
