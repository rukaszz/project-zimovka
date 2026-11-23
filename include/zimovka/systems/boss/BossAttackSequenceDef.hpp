#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSATTACKSEQUENCE_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSATTACKSEQUENCE_HPP_

#include <cstdint>
#include <optional>

#include "zimovka/systems/boss/BossAttackStep.hpp"
#include "zimovka/systems/boss/BossMovementDef.hpp"
#include "zimovka/systems/boss/BossShotDef.hpp"

namespace zimovka{
/**
 * @brief ボスの攻撃パターン遷移管理用構造体
 * 
 */
struct BossAttackSequenceDef{
    BossAttackStep step;            // 現在の攻撃パターン
    std::uint64_t duration_ticks;   // パターン持続時間
    // 攻撃ステップごとに弾の設定をもたせる
    // 攻撃ステップが無いステップもあるため，optionalを用いる
    std::optional<BossShotDef>     shot     = std::nullopt;
    std::optional<BossMovementDef> movement = std::nullopt;
    
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSATTACKSEQUENCE_HPP_
