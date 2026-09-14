#include "zimovka/math/GameplayMath.hpp"

#include <cmath>

namespace zimovka{
namespace GameplayMath{
/**
 * @brief 原点から対象を狙う角度をatan2()で計算する
 * 
 * ※atan(0, 0)=0と定義している
 * @param origin 
 * @param target 
 * @return float 
 */
float AimAngleRad(const Vec2& origin, const Vec2& target) noexcept{
    const Vec2 delta = target - origin;
    // atan(0, 0)=0であると定義
    if(delta.x == 0.0f && delta.y == 0.0f){
        return 0.0f;
    }
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
