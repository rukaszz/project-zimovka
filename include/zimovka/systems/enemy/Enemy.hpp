#ifndef ZIMOVKA_SYSTEMS_ENEMY_ENEMY_HPP_
#define ZIMOVKA_SYSTEMS_ENEMY_ENEMY_HPP_

#include <cstdint>

#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief 敵のデータ構造(コンポーネント)
 * 
 */
struct Enemy{
    // 活性/非活性で管理
    bool active = false;
    // 座標/速度
    Vec2 posotion{0, 0};
    Vec2 velocity{0, 0};
    // サイズ
    Vec2 size{32.0f, 32.0f};

    std::int32_t hp = 1;
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_ENEMY_ENEMY_HPP_
