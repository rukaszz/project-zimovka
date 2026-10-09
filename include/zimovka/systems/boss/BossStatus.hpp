#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSSTATUS_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSSTATUS_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief ボスの状態遷移管理用enum
 * 
 */
enum class BossStatus : std::uint8_t{
    Inactive,   // 非活性
    Entering,   // ボス導入
    Fighting,   // 戦闘状態
    Defeating,  // 敗北

    Count,      // 門番
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSSTATUS_HPP_
