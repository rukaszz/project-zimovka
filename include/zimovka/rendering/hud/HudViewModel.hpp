#ifndef ZIMOVKA_RENDERING_HUD_HUDVIEWMODEL_HPP_
#define ZIMOVKA_RENDERING_HUD_HUDVIEWMODEL_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief HUDに表示するデータを管理する構造体
 * 
 * ここでは各システムを持たず，スナップショットの値を受け取る
 */
struct HudViewModel{
    std::uint32_t ammo     = 0;
    std::uint32_t max_ammo = 0;

    bool reloading = false;
    std::uint32_t reload_ticks_remaining = 0;
    std::uint32_t reload_duration_ticks  = 0;

    std::uint32_t bomb_stock = 0;
};
} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_HUD_HUDVIEWMODEL_HPP_
