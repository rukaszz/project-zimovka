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
 *  Phase 1 (0-20s  tick    0-1200): 導入 - 3/5way 左右交互，FixedSpread 紹介
 *  Phase 2 (20-40s tick 1200-2400): 演出 - 交差弾・挟み撃ち・7way 初登場
 *  Phase 3 (40-60s tick 2400-3600): ラッシュ - 7way 連打・同時スポーン・高速弾
 *
 * == 角度の基準 ==
 *  VelocityFromAngle(angle, speed): x=cos(angle), y=sin(angle)
 *  π/2   (1.571) → 真下
 *  π*0.4 (1.257) → 右斜め下(72°)
 *  π*0.6 (1.885) → 左斜め下(108°)
 */
inline constexpr float PI = std::numbers::pi_v<float>;

inline constexpr std::array<EnemySpawnEvent, 30> PROTOTYPE_EVENTS{{

    // ---- Phase 1: 導入(tick 60-1080 / 1s-18s) ----
    // 3way → FixedSpread → 5way と段階的にパターンを紹介する

    {   // tick  60 ( 1s): 左 - 自機狙い 3way
        .tick = 60,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = {  0.0f,  50.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 3,
            .fire_spread_rad          = PI / 3.0f,    // 60°
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
            .fire_spread_rad          = PI / 3.0f,
            .fire_bullet_speed        = 160.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },
    {   // tick 300 ( 5s): 中央 - 自機狙い 3way (やや速)
        .tick = 300,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  52.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 3,
            .fire_spread_rad          = PI / 3.0f,
            .fire_bullet_speed        = 165.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 110,
        }
    },
    {   // tick 420 ( 7s): 左寄り - 真下 FixedSpread 5way [Fixed初登場]
        .tick = 420,
        .params = {
            .position                 = {140.0f,  80.0f},
            .velocity                 = { 20.0f,  52.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI / 2.0f,    // 真下
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,    // 90°
            .fire_bullet_speed        = 160.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },
    {   // tick 540 ( 9s): 右寄り - 真下 FixedSpread 5way
        .tick = 540,
        .params = {
            .position                 = {500.0f,  80.0f},
            .velocity                 = {-20.0f,  52.0f},
            .hp                       = 2,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI / 2.0f,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 160.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },
    {   // tick 660 (11s): 左中 - 自機狙い 5way [AimedSpread 5way 初登場]
        .tick = 660,
        .params = {
            .position                 = {200.0f,  80.0f},
            .velocity                 = {  0.0f,  55.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,    // 90°
            .fire_bullet_speed        = 170.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 100,
        }
    },
    {   // tick 780 (13s): 右中 - 自機狙い 5way
        .tick = 780,
        .params = {
            .position                 = {440.0f,  80.0f},
            .velocity                 = {  0.0f,  55.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 170.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 100,
        }
    },
    {   // tick 900 (15s): 左 - 右斜め下 FixedSpread 5way [斜め角度初登場]
        .tick = 900,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = {  0.0f,  52.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI * 0.4f,    // 72° 右斜め下
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 170.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 100,
        }
    },
    {   // tick 960 (16s): 右 - 左斜め下 FixedSpread 5way
        .tick = 960,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {  0.0f,  52.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI * 0.6f,    // 108° 左斜め下
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 170.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 100,
        }
    },
    {   // tick 1080 (18s): 中央 - 自機狙い 5way (頻度アップ)
        .tick = 1080,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  58.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 175.0f,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 90,
        }
    },

    // ---- Phase 2: 演出(tick 1200-2280 / 20s-38s) ----
    // 交差弾・挟み撃ち・7way 初登場など視覚的に面白い演出を展開

    {   // tick 1200 (20s): 左 - 右斜め下 FixedSpread 5way [交差演出の片翼]
        .tick = 1200,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = {  0.0f,  55.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI * 0.4f,    // 72° 右斜め下
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 185.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 90,
        }
    },
    {   // tick 1260 (21s): 右 - 左斜め下 FixedSpread 5way [交差演出の対翼→弾が画面中央で交差]
        .tick = 1260,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {  0.0f,  55.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI * 0.6f,    // 108° 左斜め下
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 185.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 90,
        }
    },
    {   // tick 1440 (24s): 左中 - 自機狙い 5way [挟み撃ち左]
        .tick = 1440,
        .params = {
            .position                 = {200.0f,  80.0f},
            .velocity                 = { 25.0f,  58.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 190.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 80,
        }
    },
    {   // tick 1500 (25s): 右中 - 自機狙い 5way [挟み撃ち右]
        .tick = 1500,
        .params = {
            .position                 = {440.0f,  80.0f},
            .velocity                 = {-25.0f,  58.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 190.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 80,
        }
    },
    {   // tick 1680 (28s): 中央 - 自機狙い 7way [7way 初登場]
        .tick = 1680,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  60.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,  // 120°
            .fire_bullet_speed        = 190.0f,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 90,
        }
    },
    {   // tick 1800 (30s): 左 - 真下 FixedSpread 5way [左右同時スポーン]
        .tick = 1800,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = { 30.0f,  55.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI / 2.0f,    // 真下
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 195.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 80,
        }
    },
    {   // tick 1800 (30s): 右 - 真下 FixedSpread 5way [左右同時スポーン]
        .tick = 1800,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {-30.0f,  55.0f},
            .hp                       = 3,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI / 2.0f,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 195.0f,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 80,
        }
    },
    {   // tick 2040 (34s): 左中 - 自機狙い 5way (速度アップ)
        .tick = 2040,
        .params = {
            .position                 = {220.0f,  80.0f},
            .velocity                 = { 20.0f,  60.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 200.0f,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 70,
        }
    },
    {   // tick 2160 (36s): 右中 - 自機狙い 5way
        .tick = 2160,
        .params = {
            .position                 = {420.0f,  80.0f},
            .velocity                 = {-20.0f,  60.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 200.0f,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 70,
        }
    },
    {   // tick 2280 (38s): 中央 - 真下 FixedSpread 7way [7way Fixed 初登場・Phase2 の締め]
        .tick = 2280,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  62.0f},
            .hp                       = 5,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI / 2.0f,    // 真下
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,  // 120°
            .fire_bullet_speed        = 200.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 85,
        }
    },

    // ---- Phase 3: ラッシュ(tick 2400-3300 / 40s-55s) ----
    // 7way 連打・同時スポーン・高速弾でプレッシャーをかける

    {   // tick 2400 (40s): 左 - 自機狙い 7way [7way 挟み撃ち左]
        .tick = 2400,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = { 35.0f,  62.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,
            .fire_bullet_speed        = 210.0f,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 70,
        }
    },
    {   // tick 2400 (40s): 右 - 自機狙い 7way [7way 挟み撃ち右]
        .tick = 2400,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {-35.0f,  62.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,
            .fire_bullet_speed        = 210.0f,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 70,
        }
    },
    {   // tick 2520 (42s): 中央 - 自機狙い 7way (HP増)
        .tick = 2520,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  65.0f},
            .hp                       = 5,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,
            .fire_bullet_speed        = 210.0f,
            .initial_fire_delay_ticks = 40,
            .fire_interval_ticks      = 70,
        }
    },
    {   // tick 2640 (44s): 左 - 右斜め下 FixedSpread 7way [7way 交差弾左]
        .tick = 2640,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = {  0.0f,  58.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI * 0.4f,    // 72° 右斜め下
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,
            .fire_bullet_speed        = 210.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 80,
        }
    },
    {   // tick 2700 (45s): 右 - 左斜め下 FixedSpread 7way [7way 交差弾右→弾が交差]
        .tick = 2700,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {  0.0f,  58.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI * 0.6f,    // 108° 左斜め下
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,
            .fire_bullet_speed        = 210.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 80,
        }
    },
    {   // tick 2880 (48s): 左中 - 自機狙い 5way [5way 同時スポーン左]
        .tick = 2880,
        .params = {
            .position                 = {220.0f,  80.0f},
            .velocity                 = { 25.0f,  62.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 220.0f,
            .initial_fire_delay_ticks = 40,
            .fire_interval_ticks      = 60,
        }
    },
    {   // tick 2880 (48s): 右中 - 自機狙い 5way [5way 同時スポーン右]
        .tick = 2880,
        .params = {
            .position                 = {420.0f,  80.0f},
            .velocity                 = {-25.0f,  62.0f},
            .hp                       = 4,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 5,
            .fire_spread_rad          = PI / 2.0f,
            .fire_bullet_speed        = 220.0f,
            .initial_fire_delay_ticks = 40,
            .fire_interval_ticks      = 60,
        }
    },
    {   // tick 3060 (51s): 中央 - 自機狙い 7way (最高頻度)
        .tick = 3060,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  65.0f},
            .hp                       = 5,
            .attack_pattern           = EnemyAttackPattern::AimedSpread,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,
            .fire_bullet_speed        = 220.0f,
            .initial_fire_delay_ticks = 40,
            .fire_interval_ticks      = 65,
        }
    },
    {   // tick 3300 (55s): 左 - 真下 FixedSpread 7way [最終ラッシュ左]
        .tick = 3300,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = { 40.0f,  58.0f},
            .hp                       = 5,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI / 2.0f,    // 真下
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,
            .fire_bullet_speed        = 215.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 75,
        }
    },
    {   // tick 3300 (55s): 右 - 真下 FixedSpread 7way [最終ラッシュ右]
        .tick = 3300,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {-40.0f,  58.0f},
            .hp                       = 5,
            .attack_pattern           = EnemyAttackPattern::FixedSpread,
            .fixed_fire_angle_rad     = PI / 2.0f,
            .fire_bullet_count        = 7,
            .fire_spread_rad          = PI * 2.0f / 3.0f,
            .fire_bullet_speed        = 215.0f,
            .initial_fire_delay_ticks = 50,
            .fire_interval_ticks      = 75,
        }
    },

}};

} // namespace Stage1
} // namespace zimovka

#endif  // ZIMOVKA_STAGES_STAGE1DATA_HPP_
