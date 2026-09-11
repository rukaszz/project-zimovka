#ifndef ZIMOVKA_RENDERING_SPRITE_SPRITERENDERER_HPP_
#define ZIMOVKA_RENDERING_SPRITE_SPRITERENDERER_HPP_

#include <SDL2/SDL.h>

#include "zimovka/rendering/sprite/SpriteDrawParams.hpp"
#include "zimovka/rendering/texture/Texture.hpp"

namespace zimovka{
/**
 * @brief スプライトシートを描画するクラス
 * 
 * 中心座標→描画矩形変換はSpriteRendererで一元化する
 */
class SpriteRenderer{
private:
    SDL_Renderer* renderer_ = nullptr;

public:
    // SDL_Renderer*のみを受け取るコンストラクタ
    explicit SpriteRenderer(SDL_Renderer* renderer) noexcept
        : renderer_(renderer){}
    // 描画関数
    void Draw(
        const Texture& texture, 
        const SpriteDrawParams& params
    );
    
};
} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_SPRITE_SPRITERENDERER_HPP_
