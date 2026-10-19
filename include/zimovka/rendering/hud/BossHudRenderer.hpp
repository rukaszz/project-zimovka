#ifndef ZIMOVKA_RENDERING_HUD_BOSSHUDRENDERER_HPP_
#define ZIMOVKA_RENDERING_HUD_BOSSHUDRENDERER_HPP_

#include "zimovka/rendering/PrimitiveRenderer.hpp"
#include "zimovka/rendering/hud/BossHudViewModel.hpp"

namespace zimovka{
/**
 * @brief ボスHPバーの描画を担当するクラス
 *
 */
class BossHudRenderer{
public:
    void Render(
        const BossHudViewModel& model,
        PrimitiveRenderer& primitives
    ) const;
};

} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_HUD_BOSSHUDRENDERER_HPP_
