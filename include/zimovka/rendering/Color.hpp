#ifndef ZIMOVKA_RENDERING_COLOR_HPP_
#define ZIMOVKA_RENDERING_COLOR_HPP_

#include <cstdint>

namespace zimovka{

/**
 * @brief SDL_Colorへに依存を排除するための独自定義Color
 * RGB+alphaを持ち，0〜255の範囲をとり得る
 * 安全性のため，デフォルト値で255を与える
 */
struct Color{
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;
};

}   // namespace zimovka

#endif  // ZIMOVKA_RENDERING_COLOR_HPP_
