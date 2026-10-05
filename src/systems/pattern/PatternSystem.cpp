#include "zimovka/systems/pattern/PatternSystem.hpp"

#include <cstdint>
#include <cmath>
#include <numbers>

#include "zimovka/math/GameplayMath.hpp"

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
    || pattern.spread_rad    <  0.0f
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
        pattern.base_angle_rad - pattern.spread_rad*0.5f;   // base_angle_radを中心に対称な扇状
    // パターンの進行度合い
    const float step = pattern.bullet_count > 1 ? 
        pattern.spread_rad / static_cast<float>(pattern.bullet_count - 1) : 0.0f;
    // パターン開始
    for(std::uint32_t i = 0; i < pattern.bullet_count; ++i){
        // 拡散角度
        const float angle = begin_angle + step * static_cast<float>(i);
        // 速度
        const Vec2 velocity = GameplayMath::VelocityFromAngle(angle, pattern.bullet_speed);
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

/**
 * @brief 全方向均等に弾を放射するパターン(菊型)
 *
 * EmitSpread との違い:
 *   - step = 2π / bullet_count  → 始点と終点が重複しない
 *   - spread_rad フィールドは無視する(常に全方向 2π)
 *   - base_angle_rad は開始角度のオフセットとして使用する
 *
 * @param pattern 発射パラメータ(bullet_count, origin, base_angle_rad, bullet_speed, bullet_radius を使用)
 * @param bullets 発射先の弾プール
 * @return std::size_t 実際に発射できた弾数
 */
std::size_t PatternSystem::EmitCircle(
    const PatternEmitRequest& pattern,
    BulletSystem& bullets
) const
{
    // 引数のパターンチェック(EmitSpreadと同じ基準)
    if(!std::isfinite(pattern.origin.x)
    || !std::isfinite(pattern.origin.y)
    || !std::isfinite(pattern.base_angle_rad)
    || !std::isfinite(pattern.bullet_speed)
    || !std::isfinite(pattern.bullet_radius)
    || pattern.bullet_count  == 0
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
    // 全方向を bullet_count 等分する(始点≠終点を保証)
    const float step = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(pattern.bullet_count);
    for(std::uint32_t i = 0; i < pattern.bullet_count; ++i){
        const float angle    = pattern.base_angle_rad + step * static_cast<float>(i);
        const Vec2 velocity  = GameplayMath::VelocityFromAngle(angle, pattern.bullet_speed);
        if(bullets.Spawn(pattern.origin, velocity, pattern.bullet_radius)){
            ++spawned;
        }
    }
    return spawned;
}

}   // namespace zimovka
