#ifndef ZIMOVKA_SYSTEMS_PATTERN_PATTERNSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_PATTERN_PATTERNSYSTEM_HPP_

#include <cstddef>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/pattern/PatternEmitRequest.hpp"

namespace zimovka{
/**
 * @brief 指定されたパターンで弾を生成するシステム
 * 
 */
class PatternSystem{
public:
    // 扇状弾(base_angle_radを中心に拡散)
    std::size_t EmitSpread(
        const PatternEmitRequest& pattern,
        BulletSystem& bullets
    ) const;
    // 全方向均等弾(菊型: 始点と終点が重複しない)
    std::size_t EmitCircle(
        const PatternEmitRequest& pattern,
        BulletSystem& bullets
    ) const;
};
}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_PATTERN_PATTERNSYSTEM_HPP_
