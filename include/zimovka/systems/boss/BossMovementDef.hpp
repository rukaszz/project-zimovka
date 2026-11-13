#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSMOVEMENTDEF_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSMOVEMENTDEF_HPP_

#include <cstdint>

#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief ボスの移動に関する動作の種類
 * 
 */
enum class BossMoveType{
    Stay,           // 現在位置へ留まる
    MoveTo,         // 指定した時間で目的座標へ移動
    Telepotation,   // 一定の時間をかけて目的座標へ一瞬で移動する
};

/**
 * @brief ボスの移動に関するデータ定義
 * 
 */
struct BossMovementDef{
    BossMoveType type = BossMoveType::Stay;
    Vec2 target_position{};
    std::uint32_t duration_ticks = 0;
};

} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSMOVEMENTDEF_HPP_
