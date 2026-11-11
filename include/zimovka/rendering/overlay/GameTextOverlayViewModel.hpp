#ifndef ZIMOVKA_RENDERING_OVERLAY_GAMETEXTOVERLAYVIEWMODEL_HPP_
#define ZIMOVKA_RENDERING_OVERLAY_GAMETEXTOVERLAYVIEWMODEL_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief 描画用にデータを構成するView
 * 
 * RenderPipeline→UpdatePipelineを参照しViewへデータを渡す
 */
struct GameTextOverlayViewModel{
    bool show_stage_clear = false;
    std::uint64_t stage_clear_duration_ticks  = 0; // クリア演出時間
    std::uint64_t stage_clear_ticks_remaining = 0; // クリア演出のカウントダウン

    bool show_warning     = false;
    std::uint64_t warning_duration_ticks  = 0;     // Warning演出時間
    std::uint64_t warning_ticks_remaining = 0;     // Warning演出のカウントダウン

}; 
} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_OVERLAY_GAMETEXTOVERLAYVIEWMODEL_HPP_
