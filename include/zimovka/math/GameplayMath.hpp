#ifndef ZIMOVKA_MATH_GAMEPLAYMATH_HPP_
#define ZIMOVKA_MATH_GAMEPLAYMATH_HPP_

#include <cstdint>

#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief Gameplayで使用する数学関数の境界API
 *
 * 現在は標準数学ライブラリを使用する。
 * 将来的にリプレイなどでクロスプラットホーム対応が必要になった場合に
 * LUT / fixed-point等へ実装を置換するための境界
 */
namespace GameplayMath{
// GameplayMathのバージョン管理用
inline constexpr std::uint32_t VERSION = 1;

// atan2による自機狙い用
[[nodiscard]]
float AimAngleRad(const Vec2& origin, const Vec2& target) noexcept;

// cos/sinを利用した弾速計算用
[[nodiscard]]
Vec2 VelocityFromAngle(float angle_rad, float speed) noexcept;

}   // namespace GameplayMath
}   // namespace zimovka


#endif  // ZIMOVKA_MATH_GAMEPLAYMATH_HPP_
