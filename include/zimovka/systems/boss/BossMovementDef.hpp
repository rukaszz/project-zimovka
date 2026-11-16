#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSMOVEMENTDEF_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSMOVEMENTDEF_HPP_

#include <cstdint>

#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief ボスの移動動作の種類
 * 
 */
enum class BossMoveType{
    Stay,           // 現在位置へ留まる
    MoveTo,         // 指定した時間で目的座標へ移動
    Telepotation,   // 一定の時間をかけて目的座標へ一瞬で移動する
};

/**
 * @brief ボスの移動に対する起点
 * 
 */
enum class BossMoveTarget : std::uint8_t{
    Fixed,          // スクリプト移動(target_positionを用いる)
    Random,         // move_range内でランダムに移動する
    OppositePlayer, // プレイヤーの反対側へ移動する(x軸は反転・Y軸は上方向)
    FacePlayer      // プレイヤーの正面へ移動する
};

/**
 * @brief ボスの移動に関するデータ定義
 * 
 */
struct BossMovementDef{
    BossMoveType type = BossMoveType::Stay;
    BossMoveTarget target_mode = BossMoveTarget::Fixed;
    Vec2 target_position{};             // Fixedで使用
    Vec2 move_range{};                  // 移動の制限(Randomなどで使用)：c±range
    std::uint32_t duration_ticks = 0;   // 移動が完了するまでの時間
};

} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSMOVEMENTDEF_HPP_
