#ifndef ZIMOVKA_RENDERING_COLOR_HPP_
#define ZIMOVKA_RENDERING_COLOR_HPP_

#include <cstdint>

namespace zimovka{

/**
 * @brief SDL_Colorへに依存を排除するための独自定義Color
 * RGB+alphaを持ち，0〜255の範囲をとり得る
 * 
 */
struct Color{
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;
};

}   // namespace zimovka

#endif  // ZIMOVKA_RENDERING_COLOR_HPP_
