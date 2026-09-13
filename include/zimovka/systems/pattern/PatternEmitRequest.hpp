#ifndef ZIMOVKA_SYSTEMS_PATTERN_PATTERNEMITREQUEST_HPP_
#define ZIMOVKA_SYSTEMS_PATTERN_PATTERNEMITREQUEST_HPP_

#include <cstdint>

#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief 発射要求する弾の各種項目の定義
 * 
 */
struct PatternEmitRequest{
    Vec2 origin{};                      // 原点座標
    float base_angle_rad       = 0.0f;  // 基準の傾き
    std::uint32_t bullet_count = 0;     // パターンが持つ弾数
    // 弾のパラメータ
    float spread_rad    = 0.50f;
    float bullet_speed  = 180.0f;
    float bullet_radius = 3.0f;
};
}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_PATTERN_PATTERNEMITREQUEST_HPP_
