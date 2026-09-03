#include "zimovka/systems/pattern/PatternSystem.hpp"

#include <cstdint>
#include <cmath>

namespace zimovka{
/**
 * @brief 放射状に広がるパターン
 * 
 * @param pattern 
 * @param bullets 
 * @return std::size_t 
 */
std::size_t PatternSystem::EmitSpread(
    const PatternEmitRequest& pattern, 
    BulletSystem& bullets
) const
{
    // 引数のパターンチェック
    if(!std::isfinite(pattern.origin.x)
    || !std::isfinite(pattern.origin.y)
    || !std::isfinite(pattern.base_angle_rad)
    || !std::isfinite(pattern.spread_rad)
    || !std::isfinite(pattern.bullet_speed)
    || !std::isfinite(pattern.bullet_radius)
    || pattern.bullet_count  == 0
    || pattern.spread_rad    < 0.0f
    || pattern.bullet_speed  <= 0.0f
    || pattern.bullet_radius <= 0.0f)
    {
        return 0;
    }
    // 上限値に対して発射可能か
    const std::size_t available = bullets.GetCapacity() - bullets.CountActive();
    if(available < pattern.bullet_count){
        return 0;
    }
    // 出現数
    std::size_t spawned = 0;
    // 開始地点の角度
    const float begin_angle = 
        pattern.base_angle_rad - pattern.spread_rad*0.5f;   // base_angle_radを中心に対象的な5way
    // パターンの進行度合い
    const float step = pattern.bullet_count > 1 ? 
        pattern.spread_rad / static_cast<float>(pattern.bullet_count - 1) : 0.0f;
    // パターン開始
    for(std::uint32_t i = 0; i < pattern.bullet_count; ++i){
        // 拡散角度
        const float angle = begin_angle + step * static_cast<float>(i);
        // 速度
        const Vec2 velocity{
            std::cos(angle) * pattern.bullet_speed, // x軸
            std::sin(angle) * pattern.bullet_speed  // y軸
        };
        // 上記で設定した角度・速度で弾を出現
        if(bullets.Spawn(
            pattern.origin, velocity, pattern.bullet_radius
        ))
        {
            ++spawned;
        }
    }
    return spawned;
}

}   // namespace zimovka
