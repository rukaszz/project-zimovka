#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSS_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSS_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "zimovka/core/Circle.hpp"
#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/boss/BossAttackStep.hpp"
#include "zimovka/systems/boss/BossAttackSequenceDef.hpp"
#include "zimovka/systems/boss/BossMovementDef.hpp"
#include "zimovka/systems/boss/BossStatus.hpp"
#include "zimovka/systems/boss/BossPhaseDef.hpp"

namespace zimovka{
/**
 * @brief Boss関係のデータを管理する構造体
 *
 */
struct Boss{
    // 名前
    std::string name = "";
    // ボスの状態
    BossStatus status = BossStatus::Inactive;
    BossAttackStep attack_step = BossAttackStep::AimedPressure;

    Vec2 position{};                // 中心座標
    Vec2 velocity{};                // 速度
    Vec2 render_size{};             // 描画サイズ

    // 移動関連の要素
    Vec2          move_start{};
    Vec2          move_target{};
    std::uint32_t move_duration_ticks = 0;
    std::uint32_t move_elapsed_ticks  = 0;
    BossMoveType  move_type = BossMoveType::Stay;   // Bossの移動ロジック区別用

    float hurtbox_radius = 0.0f;    // 当たり判定半径
    float contact_radius = 0.0f;    // 自機との衝突判定半径

    std::int32_t hp      = 0;       // 現体力

    std::uint64_t entering_ticks_remaining    = 0; // 登場フェーズのカウントダウン
    std::uint64_t defeating_duration_ticks    = 0; // 撃破演出の総tick数(描画のscale計算用)
    std::uint64_t defeating_ticks_remaining   = 0; // 敗北フェーズのカウントダウン

    // ボスの攻撃パターン切り替え定義
    std::size_t   attack_step_index           = 0; // 攻撃パターン切り替え用添字
    std::uint64_t attack_step_ticks_remaining = 0; // 戦闘状態遷移のカウントダウン

    // ボスのフェーズを管理するためのインデックス
    std::size_t phase_index = 0;
    // ボスのフェーズ遷移判定用
    bool phase_transition_pending = false;
    // ボスの行動パターン設定用フェーズ管理vector
    std::vector<BossPhaseDef> phases{};

    std::uint32_t fire_timer     = 0;        // 発射間隔制御用タイマ

    // CollisionSystemから参照するための当たり判定円
    Circle GetHurtboxCircle() const noexcept{
        return Circle{position, hurtbox_radius};
    }
    Circle GetContactCircle() const noexcept{
        return Circle{position, contact_radius};
    }
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSS_HPP_
