#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSATTACKSTEP_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSATTACKSTEP_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief fighting状態のボスの状態管理用enum
 * 
 */
enum class BossAttackStep : std::int8_t{
    AimedPressure, 
    SpecialWarning, 
    CircleBarrage, 
    Recovery, 
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSATTACKSTEP_HPP_
