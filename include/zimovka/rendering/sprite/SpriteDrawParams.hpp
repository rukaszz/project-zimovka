#ifndef ZIMOVKA_RENDERING_SPRITE_SPRITEDRAWPARAMS_HPP_
#define ZIMOVKA_RENDERING_SPRITE_SPRITEDRAWPARAMS_HPP_

#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief スプライト描画変数管理用構造体
 * 
 */
struct SpriteDrawParams{
    Vec2 center{};
    Vec2 size{};

    double angle_deg = 0.0;
    bool flip_x = false;
};  
} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_SPRITE_SPRITEDRAWPARAMS_HPP_
