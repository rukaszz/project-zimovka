#ifndef ZIMOVKA_STAGES_STAGE1DATA_HPP_
#define ZIMOVKA_STAGES_STAGE1DATA_HPP_

#include <array>
#include <cstddef>
#include <numbers>

#include "zimovka/events/EnemySpawnEvent.hpp"
#include "zimovka/systems/enemy/EnemyAttackPattern.hpp"

namespace zimovka{
namespace Stage1{

/**
 * @brief ステージ1 敵スポーンイベント配列
 *
 * tick は 60Hz(simulation)基準(tick 60 = 1s)
 * X 座標はプレイフィールド幅 640px に収まる範囲で設定
 * tickの昇順で各要素を並べること(EnemySpawner::UpdateTickの前提条件)
 *
 * == フェーズ構成 ==
 *  Phase 1 (0-10s tick   0- 600): 導入 - 左右交互に3way/5way自機狙い
 *  Phase 2 (10-20s tick 600-1200): 中盤 - FixedSpread 混在、弾速/頻度アップ
 *  Phase 3 (20-30s tick 1200-1800): 終盤 - 同時出現、7way、高頻度
 *
 * == 角度の基準 ==
 *  VelocityFromAngle(angle, speed): x=cos(angle), y=sin(angle)
 *  π/2  (1.571) → 真下
 *  π*0.4 (1.257) → 右斜め下(72°)
 *  π*0.6 (1.885) → 左斜め下(108°)
 */
inline constexpr float kPi = std::numbers::pi_v<float>;

inline constexpr std::array<EnemySpawnEvent, 20> PROTOTYPE_EVENTS{{

    // ---- Phase 1: 導入(tick 0-600 / 0-10s) ----

    {   // tick  60 ( 1s): 左 - 自機狙い 3way
        .tick = 60,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = {  0.0f,  50.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 3,
            .fire_spread_rad          = kPi / 3.0f,    // 60°
            .fire_bullet_speed        = 160.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },
    {   // tick 180 ( 3s): 右 - 自機狙い 3way
        .tick = 180,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {  0.0f,  50.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 3,
            .fire_spread_rad          = kPi / 3.0f,
            .fire_bullet_speed        = 160.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },
    {   // tick 300 ( 5s): 中央 - 自機狙い 5way
        .tick = 300,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  55.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,    // 90°
            .fire_bullet_speed        = 170.0f,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 100,
        }
    },
    {   // tick 420 ( 7s): 左寄り - 真下 FixedSpread 5way
        .tick = 420,
        .params = {
            .position                 = {130.0f,  80.0f},
            .velocity                 = { 20.0f,  50.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = kPi / 2.0f,    // 真下
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 160.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },
    {   // tick 540 ( 9s): 右寄り - 真下 FixedSpread 5way
        .tick = 540,
        .params = {
            .position                 = {510.0f,  80.0f},
            .velocity                 = {-20.0f,  50.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = kPi / 2.0f,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 160.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },

    // ---- Phase 2: 中盤(tick 600-1200 / 10-20s) ----

    {   // tick 600 (10s): 左中 - 自機狙い 3way (速度アップ)
        .tick = 600,
        .params = {
            .position                 = {200.0f,  80.0f},
            .velocity                 = { 30.0f,  55.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 3,
            .fire_spread_rad          = kPi / 3.0f,
            .fire_bullet_speed        = 190.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 100,
        }
    },
    {   // tick 660 (11s): 右中 - 自機狙い 3way (速度アップ)
        .tick = 660,
        .params = {
            .position                 = {440.0f,  80.0f},
            .velocity                 = {-30.0f,  55.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 3,
            .fire_spread_rad          = kPi / 3.0f,
            .fire_bullet_speed        = 190.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 100,
        }
    },
    {   // tick 780 (13s): 中央 - 自機狙い 5way (HP増・頻度アップ)
        .tick = 780,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  65.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 190.0f,
            .initial_fire_delay_ticks = 40,
            .fire_interval_ticks      = 80,
        }
    },
    {   // tick 900 (15s): 左 - 右斜め下 FixedSpread 5way
        .tick = 900,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = {  0.0f,  50.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = kPi * 0.4f,    // 72° (右斜め下)
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 180.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 100,
        }
    },
    {   // tick 960 (16s): 右 - 左斜め下 FixedSpread 5way
        .tick = 960,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {  0.0f,  50.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = kPi * 0.6f,    // 108° (左斜め下)
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 180.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 100,
        }
    },
    {   // tick 1080 (18s): 左 - 自機狙い 5way (高速)
        .tick = 1080,
        .params = {
            .position                 = {120.0f,  80.0f},
            .velocity                 = { 30.0f,  60.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 200.0f,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 80,
        }
    },
    {   // tick 1140 (19s): 右 - 自機狙い 5way (高速)
        .tick = 1140,
        .params = {
            .position                 = {520.0f,  80.0f},
            .velocity                 = {-30.0f,  60.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 200.0f,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 80,
        }
    },

    // ---- Phase 3: 終盤(tick 1200-1800 / 20-30s) ----

    {   // tick 1200 (20s): 中央 - 自機狙い 7way (HP増)
        .tick = 1200,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  70.0f},
            .hp                       = 5,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = kPi * 2.0f / 3.0f,  // 120°
            .fire_bullet_speed        = 200.0f,
            .initial_fire_delay_ticks = 40,
            .fire_interval_ticks      = 70,
        }
    },
    {   // tick 1260 (21s): 左 - 真下 FixedSpread 7way
        .tick = 1260,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = { 40.0f,  55.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = kPi / 2.0f,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = kPi * 2.0f / 3.0f,
            .fire_bullet_speed        = 185.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 90,
        }
    },
    {   // tick 1320 (22s): 右 - 真下 FixedSpread 7way
        .tick = 1320,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {-40.0f,  55.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = kPi / 2.0f,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = kPi * 2.0f / 3.0f,
            .fire_bullet_speed        = 185.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 90,
        }
    },
    {   // tick 1440 (24s): 左中 - 自機狙い 5way (頻度アップ)
        .tick = 1440,
        .params = {
            .position                 = {240.0f,  80.0f},
            .velocity                 = { 20.0f,  55.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 210.0f,
            .initial_fire_delay_ticks = 40,
            .fire_interval_ticks      = 70,
        }
    },
    {   // tick 1500 (25s): 右中 - 自機狙い 5way (頻度アップ)
        .tick = 1500,
        .params = {
            .position                 = {400.0f,  80.0f},
            .velocity                 = {-20.0f,  55.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 210.0f,
            .initial_fire_delay_ticks = 40,
            .fire_interval_ticks      = 70,
        }
    },
    {   // tick 1620 (27s): 左 - 自機狙い 5way (最高速)
        .tick = 1620,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = {  0.0f,  60.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 220.0f,
            .initial_fire_delay_ticks = 35,
            .fire_interval_ticks      = 65,
        }
    },
    {   // tick 1680 (28s): 右 - 自機狙い 5way (最高速)
        .tick = 1680,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {  0.0f,  60.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = kPi / 2.0f,
            .fire_bullet_speed        = 220.0f,
            .initial_fire_delay_ticks = 35,
            .fire_interval_ticks      = 65,
        }
    },
    {   // tick 1740 (29s): 中央 - 自機狙い 7way (ラストラッシュ)
        .tick = 1740,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  55.0f},
            .hp                       = 6,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = kPi * 2.0f / 3.0f,
            .fire_bullet_speed        = 220.0f,
            .initial_fire_delay_ticks = 35,
            .fire_interval_ticks      = 60,
        }
    },

}};

} // namespace Stage1
} // namespace zimovka

#endif  // ZIMOVKA_STAGES_STAGE1DATA_HPP_
