#ifndef ZIMOVKA_SYSTEMS_PLAYER_PLAYER_HPP_
#define ZIMOVKA_SYSTEMS_PLAYER_PLAYER_HPP_

#include "zimovka/core/Vec2.hpp"

namespace zimovka{

/**
 * @brief プレイヤーの情報を管理する構造体
 * 
 */
struct Player{
    // 座標
    Vec2 position{480.0f, 600.0f};
    // 通常移動速度
    float normal_speed = 320.0f;
    // 低速移動速度
    float slow_speed = 140.0f;
    // プレイヤーの幅/高さ
    float width = 24.0f;
    float height = 24.0f;
    // 当たり判定半径(当たり判定は円)
    float hit_radius = 4.0f;
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_PLAYER_PLAYER_HPP_
