#ifndef ZIMOVKA_SYSTEMS_BULLET_BULLET_HPP_
#define ZIMOVKA_SYSTEMS_BULLET_BULLET_HPP_

#include "zimovka/core/Vec2.hpp"
#include "zimovka/rendering/Color.hpp"

namespace zimovka{

/**
 * @brief 弾のデータ構造(コンポーネント)
 * 
 */
struct Bullet{
    // 活性非活性管理用
    bool active = false;
    // 座標
    Vec2 position{};    // x, y
    // 2次元の速度
    Vec2 velocity{};    // px/s
    // 弾半径
    float radius = 4.0f;
    // 弾の色
    Color color{};
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BULLET_BULLET_HPP_
