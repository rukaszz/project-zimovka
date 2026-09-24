#ifndef ZIMOVKA_RENDERING_HUD_HUDVIEWMODEL_HPP_
#define ZIMOVKA_RENDERING_HUD_HUDVIEWMODEL_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief HUDに表示するデータを管理する構造体
 * 
 * ここでは各システムを持たず，スナップショットの値を受け取る
 * →HUD周りではシステム側のデータを変えない
 */
struct HudViewModel{
    std::uint32_t ammo     = 0;
    std::uint32_t max_ammo = 0;

    bool  reloading       = false;
    float reload_progress = 0.0f;  // [0.0, 1.0] RenderPipeline でclamp済み

    std::uint32_t bomb_stock = 0;
};
} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_HUD_HUDVIEWMODEL_HPP_
