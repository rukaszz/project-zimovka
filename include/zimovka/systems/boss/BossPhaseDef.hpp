#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSPHASEDEF_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSPHASEDEF_HPP_

#include <cstdint>
#include <vector>

#include "zimovka/systems/boss/BossAttackSequenceDef.hpp"

namespace zimovka{
/**
 * @brief ボスのHPに対するフェーズ
 * 
 */
enum class BossPhaseKind : std::uint8_t{
    Normal, 
    Spell, 
};

/**
 * @brief ボスのHPに対するフェーズ遷移用の定義
 * 
 */
struct BossPhaseDef{
    BossPhaseKind kind = BossPhaseKind::Normal;

    std::int32_t max_hp = 1;
    // このHP以下でフェーズ終了(現在のフェーズのHPに対する絶対値)
    std::int32_t clear_at_hp = 0;

    // ここでAttackSequenceも管理
    std::vector<BossAttackSequenceDef> attack_sequence;

};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSPHASEDEF_HPP_
