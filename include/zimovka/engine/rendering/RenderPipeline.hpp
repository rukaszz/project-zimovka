#ifndef ZIMOVKA_ENGINE_RENDERING_RENDERPIPELINE_HPP_
#define ZIMOVKA_ENGINE_RENDERING_RENDERPIPELINE_HPP_

#include <SDL2/SDL.h>

#include "zimovka/engine/update/UpdatePipeline.hpp"
#include "zimovka/rendering/Renderer.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"
#include "zimovka/rendering/overlay/GameTextOverlay.hpp"
#include "zimovka/rendering/sprite/SpriteRenderer.hpp"
#include "zimovka/rendering/texture/TextureStore.hpp"

namespace zimovka{
/**
 * @brief 1描画フレームの描画順序を管理する
 *
 */
class RenderPipeline{
private:
    GameTextOverlay text_overlay_;  // TTF_Fontを所有する

public:
    // フォントの読み込み等を含めた初期化
    void Initialize(SDL_Renderer* renderer, const std::string& font_path);

    void RenderFrame(
        const UpdatePipeline& gameplay,
        Renderer&             renderer,
        SpriteRenderer&       sprites,
        PrimitiveRenderer&    primitives,
        const TextureStore&   textures,
        bool draw_debug_collision
    );
};
} // namespace zimovka

#endif  // ZIMOVKA_ENGINE_RENDERING_RENDERPIPELINE_HPP_
