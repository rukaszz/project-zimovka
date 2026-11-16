#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSCONFIG_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSCONFIG_HPP_

#include <cstdint>
#include <numbers>
#include <optional>
#include <string>
#include <vector>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/boss/BossAttackSequenceDef.hpp"
#include "zimovka/systems/boss/BossPhaseDef.hpp"
#include "zimovka/systems/boss/BossShotDef.hpp"

namespace zimovka{
/**
 * @brief BossSystemでボスを管理する際に渡す情報管理用構造体
 * 
 * ここではデフォルト値を設定しておく
 */
struct BossConfig{
    std::string name = "boss 1";
    Vec2 position{320.0f, 100.0f};
    Vec2 velocity{0.0f, 5.0f};
    Vec2 render_size{128.0f, 128.0f};

    float hurtbox_radius = 40.0f;
    float contact_radius = 24.0f;

    std::uint64_t entering_duration_ticks    = 180;  // 開始演出時間
    std::uint64_t defeating_duration_ticks   = 180;  // 撃破演出時間
    std::uint32_t aimed_fire_interval        = 90;   // 自機狙い発射間隔(tick)
    std::uint32_t circle_fire_interval       = 60;   // 全方向発射間隔(tick)

    // ボスの攻撃パターン切り替え定義
    std::vector<BossPhaseDef> phases{
        { // Phase1 (Normal)
            BossPhaseKind::Normal,  // kind
            20, 5,                  // max_hp, clear_at_hp
            {   // vector<BossAttackSequenceDef>
                {   // 自機狙い拡散弾: プレイヤーの反対側へ移動しながら攻撃
                    BossAttackStep::AimedPressure, 180,
                    BossShotDef{5, 90, 200.0f, 3.0f, std::numbers::pi_v<float> / 2.0f},
                    BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::OppositePlayer, {}, {}, 60}
                },
                {   // フェーズ移行警告: 画面中央上へ移動
                    BossAttackStep::SpecialWarning, 90,
                    std::nullopt,
                    BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::Fixed, {320.0f, 100.0f}, {}, 60}
                },
                {   // 全包囲弾: ランダムに逃げながら攻撃
                    BossAttackStep::CircleBarrage, 180,
                    BossShotDef{12, 60, 150.0f, 4.0f, 0.0f},
                    BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::Random, {}, {120.0f, 60.0f}, 90}
                },
                {   // リカバリー: 移動なし
                    BossAttackStep::Recovery,       75,
                    std::nullopt, std::nullopt
                }
            }
        },
        { // Phase2 (Spell)
            BossPhaseKind::Spell,   // kind
            15, 0,                  // max_hp, clear_at_hp
            {   // vector<BossAttackSequenceDef>
                {   // スペルカード警告: 画面中央上へ移動
                    BossAttackStep::SpecialWarning, 90,
                    std::nullopt,
                    BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::Fixed, {320.0f, 80.0f}, {}, 60}
                },
                {   // 全包囲弾: プレイヤーの正面へ移動しながら攻撃
                    BossAttackStep::CircleBarrage, 180,
                    BossShotDef{16, 45, 200.0f, 4.0f, 0.0f},
                    BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::FacePlayer, {}, {}, 90}
                },
                {   // リカバリー: 移動なし
                    BossAttackStep::Recovery,       60,
                    std::nullopt, std::nullopt
                }
            }
        }
    };
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSCONFIG_HPP_
