#ifndef ZIMOVKA_CORE_CIRCLE_HPP_
#define ZIMOVKA_CORE_CIRCLE_HPP_

#include "zimovka/core/Vec2.hpp"

namespace zimovka{

/**
 * @brief 円のデータ構造の定義
 * 
 */
struct Circle{
    Vec2 center{};
    float radius = 0.0f;
};

}   // namespace zimovka

#endif  // ZIMOVKA_CORE_CIRCLE_HPP_
