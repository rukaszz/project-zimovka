#ifndef ZIMOVKA_SYSTEMS_PATTERN_PATTERNSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_PATTERN_PATTERNSYSTEM_HPP_

#include <cstddef>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/pattern/PatternDefinition.hpp"

namespace zimovka{
/**
 * @brief 弾幕パターンを管理するシステム
 * 
 */
class PatternSystem{
public:
    std::size_t EmitSpread(
        const PatternDefinition& pattern, 
        BulletSystem& bullets
    );
};
}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_PATTERN_PATTERNSYSTEM_HPP_
