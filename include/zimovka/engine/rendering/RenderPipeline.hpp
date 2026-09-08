#ifndef ZIMOVKA_ENGINE_RENDERING_RENDERPIPELINE_HPP_
#define ZIMOVKA_ENGINE_RENDERING_RENDERPIPELINE_HPP_

#include "zimovka/engine/update/UpdatePipeline.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"
#include "zimovka/rendering/SpriteRenderer.hpp"
#include "zimovka/rendering/TextureStore.hpp"

namespace zimovka{
/**
 * @brief 1固定Tickあたりの更新順序を管理し各システムの描画関数を呼び出す
 * 
 */
class RenderPipeline{
public:
    void RenderTick(
        const UpdatePipeline& gameplay, 
        SpriteRenderer&       sprites, 
        PrimitiveRenderer&    primitives, 
        const TextureStore&   texutures, 
        bool draw_debug_collision
    ) const;
}; 
} // namespace zimovka

#endif  // ZIMOVKA_ENGINE_RENDERING_RENDERPIPELINE_HPP_
