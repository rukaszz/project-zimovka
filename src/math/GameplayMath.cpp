#include "zimovka/math/GameplayMath.hpp"

#include <cmath>

namespace zimovka{
namespace GameplayMath{
/**
 * @brief 原点から対象を狙う角度をatan2()で計算する
 * 
 * @param origin 
 * @param target 
 * @return float 
 */
float AimAngleRad(const Vec2& origin, const Vec2& target) noexcept{
    const Vec2 delta = target - origin;
    return std::atan2(delta.y, delta.x);
}

/**
 * @brief 三角関数を用いた弾速計算
 * 
 * @param angle_rad 
 * @param speed 
 * @return Vec2 
 */
Vec2 VelocityFromAngle(float angle_rad, float speed) noexcept{
    return{
        std::cos(angle_rad) * speed,    // x
        std::sin(angle_rad) * speed     // y
    };
}

}   // namespace GameplayMath
}   // zimovka
