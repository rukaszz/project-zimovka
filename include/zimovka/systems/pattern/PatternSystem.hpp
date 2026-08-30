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
    // 扇状弾
    std::size_t EmitSpread(
        const PatternEmitRequest& pattern, 
        BulletSystem& bullets
    ) const noexcept;
};
}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_PATTERN_PATTERNSYSTEM_HPP_
