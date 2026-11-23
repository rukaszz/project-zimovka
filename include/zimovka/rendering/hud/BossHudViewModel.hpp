#ifndef ZIMOVKA_RENDERING_HUD_BOSSHUDVIEWMODEL_HPP_
#define ZIMOVKA_RENDERING_HUD_BOSSHUDVIEWMODEL_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief ボスのHPバーを描画に関する情報
 * 
 */
struct BossHudViewModel{
    bool visible = false;
    std::int32_t hp          = 0;
    std::int32_t clear_at_hp = 0;   // フェーズ切り替え用基準値
    std::int32_t max_hp      = 0;
};    
} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_HUD_BOSSHUDVIEWMODEL_HPP_
