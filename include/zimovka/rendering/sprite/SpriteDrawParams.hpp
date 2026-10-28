#ifndef ZIMOVKA_RENDERING_SPRITE_SPRITEDRAWPARAMS_HPP_
#define ZIMOVKA_RENDERING_SPRITE_SPRITEDRAWPARAMS_HPP_

#include <cstdint>

#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief スプライト描画変数管理用構造体
 *
 */
struct SpriteDrawParams{
    Vec2          center{};
    Vec2          size{};
    double        angle_deg = 0.0;
    bool          flip_x    = false;
    std::uint8_t  alpha     = 255;  // 0=透明, 255=不透明(デフォルト)
};  
} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_SPRITE_SPRITEDRAWPARAMS_HPP_
