#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSCONFIG_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSCONFIG_HPP_

#include <cstdint>
#include <string>

#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief BossSystemでボスを管理する際に渡す情報管理用構造体
 * 
 * ここではデフォルト値を設定しておく
 */
struct BossConfig{
    std::string name = "boss 1";
    Vec2 position{480.0f, 100.0f};
    Vec2 velocity{0.0f, 5.0f};
    Vec2 render_size{128.0f, 128.0f};

    float hurtbox_radius = 40.0f;
    float contact_radius = 24.0f;

    std::int32_t max_hp = 20;

    std::uint64_t fight_tick = 3600;

    std::uint32_t aimed_fire_interval  = 90;   // 自機狙い発射間隔(tick)
    std::uint32_t circle_fire_interval = 180;  // 全方向発射間隔(tick)
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSCONFIG_HPP_
