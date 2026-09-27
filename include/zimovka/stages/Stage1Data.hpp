#ifndef ZIMOVKA_STAGES_STAGE1DATA_HPP_
#define ZIMOVKA_STAGES_STAGE1DATA_HPP_

#include <array>
#include <cstddef>

#include "zimovka/events/EnemySpawnEvent.hpp"

namespace zimovka{
namespace Stage1{

/**
 * @brief プロトタイプ確認用スポーンイベント配列
 *
 * tickは60Hz(simulation)基準(tick 60 = 1s)
 * X座標はプレイフィールド幅640pxに収まる範囲で設定
 *
 * tickの昇順で各要素を並べること(EnemySpawner::UpdateTickの前提条件)
 */
inline constexpr std::array<EnemySpawnEvent, 5> PROTOTYPE_EVENTS{{
    {   // tick 60 (1秒): 左側から出現
        .tick = 60,
        .params = {
            .position                 = {160.0f,  80.0f},
            .velocity                 = {  0.0f,  50.0f},
            .render_size              = { 32.0f,  32.0f},
            .hurtbox_radius           = 13.0f,
            .contact_radius           = 10.0f,
            .hp                       = 2,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },
    {   // tick 180 (3秒): 右側から出現
        .tick = 180,
        .params = {
            .position                 = {480.0f,  80.0f},
            .velocity                 = {  0.0f,  50.0f},
            .render_size              = { 32.0f,  32.0f},
            .hurtbox_radius           = 13.0f,
            .contact_radius           = 10.0f,
            .hp                       = 2,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },
    {   // tick 300 (5秒): 中央から出現 HP多め・速い
        .tick = 300,
        .params = {
            .position                 = {320.0f,  80.0f},
            .velocity                 = {  0.0f,  60.0f},
            .render_size              = { 32.0f,  32.0f},
            .hurtbox_radius           = 13.0f,
            .contact_radius           = 10.0f,
            .hp                       = 3,
            .initial_fire_delay_ticks = 30,
            .fire_interval_ticks      = 90,
        }
    },
    {   // tick 420 (7秒): 左寄りから出現
        .tick = 420,
        .params = {
            .position                 = {240.0f,  80.0f},
            .velocity                 = {  0.0f,  40.0f},
            .render_size              = { 32.0f,  32.0f},
            .hurtbox_radius           = 13.0f,
            .contact_radius           = 10.0f,
            .hp                       = 2,
            .initial_fire_delay_ticks = 60,
            .fire_interval_ticks      = 120,
        }
    },
    {   // tick 600 (10秒): 右寄りから出現 HP多め
        .tick = 600,
        .params = {
            .position                 = {400.0f,  80.0f},
            .velocity                 = {  0.0f,  70.0f},
            .render_size              = { 32.0f,  32.0f},
            .hurtbox_radius           = 13.0f,
            .contact_radius           = 10.0f,
            .hp                       = 4,
            .initial_fire_delay_ticks = 45,
            .fire_interval_ticks      = 100,
        }
    },
}};

} // namespace Stage1
} // namespace zimovka

#endif  // ZIMOVKA_STAGES_STAGE1DATA_HPP_
