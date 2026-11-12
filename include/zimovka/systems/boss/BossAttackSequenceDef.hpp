#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSATTACKSEQUENCE_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSATTACKSEQUENCE_HPP_

#include <cstdint>

#include "zimovka/systems/boss/BossAttackStep.hpp"

namespace zimovka{
/**
 * @brief ボスの攻撃パターン遷移管理用構造体
 * 
 */
struct BossAttackSequenceDef{
    BossAttackStep step;            // 現在の攻撃パターン
    std::uint64_t duration_ticks;   // パターン持続時間
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSATTACKSEQUENCE_HPP_
