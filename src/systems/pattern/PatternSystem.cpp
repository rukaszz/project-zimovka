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
    const PatternDefinition& pattern, 
    BulletSystem& bullets
)
{
    // 弾数チェック
    if(pattern.bullet_count_ == 0){
        return 0;
    }
    // 出現数
    std::size_t spawned = 0;
    // 開始地点の角度
    const float begin_angle = 
        pattern.base_angle_rad - pattern.spread_rad + 0.5f;
    // パターンの進行度合い
    const float step = pattern.bullet_count_ > 1 ? 
        pattern.spread_rad / static_cast<float>(pattern.bullet_count_ - 1) : 0.0f;
    // パターン開始
    for(std::uint32_t i = 0; i < pattern.bullet_count_; ++i){
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
