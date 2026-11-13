#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSSHOTDEF_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSSHOTDEF_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief ボス弾の設定管理用構造体
 * 
 */
struct BossShotDef{
    std::uint32_t bullet_count       = 0;
    std::uint32_t fire_interval_tick = 0;

    float bullet_speed  = 0.0f;
    float bullet_radius = 0.0f;
    float spread_rad    = 0.0f;
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSSHOTDEF_HPP_
