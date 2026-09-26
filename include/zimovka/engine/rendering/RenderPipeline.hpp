#ifndef ZIMOVKA_ENGINE_RENDERING_RENDERPIPELINE_HPP_
#define ZIMOVKA_ENGINE_RENDERING_RENDERPIPELINE_HPP_

#include "zimovka/engine/update/UpdatePipeline.hpp"
#include "zimovka/rendering/Renderer.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"
#include "zimovka/rendering/sprite/SpriteRenderer.hpp"
#include "zimovka/rendering/texture/TextureStore.hpp"

namespace zimovka{
/**
 * @brief 1描画フレームの描画順序を管理する
 *
 */
class RenderPipeline{
public:
    void RenderFrame(
        const UpdatePipeline& gameplay,
        Renderer&             renderer,
        SpriteRenderer&       sprites,
        PrimitiveRenderer&    primitives,
        const TextureStore&   textures,
        bool draw_debug_collision
    ) const;
};
} // namespace zimovka

#endif  // ZIMOVKA_ENGINE_RENDERING_RENDERPIPELINE_HPP_
