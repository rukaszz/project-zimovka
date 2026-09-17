#ifndef ZIMOVKA_RENDERING_HUD_HUDRENDERER_HPP_
#define ZIMOVKA_RENDERING_HUD_HUDRENDERER_HPP_

#include "zimovka/rendering/PrimitiveRenderer.hpp"
#include "zimovka/rendering/hud/HudViewModel.hpp"

namespace zimovka{
/**
 * @brief HUD描画関係の処理を管理するクラス
 * 
 * NOTE: 最初はPrimitiveRendererを用いてHUDの骨格を作る
 */
class HudRenderer{
public:
    void Render(
        const HudViewModel& model,
        PrimitiveRenderer& primitives
    ) const;
};

} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_HUD_HUDRENDERER_HPP_
