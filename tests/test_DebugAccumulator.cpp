#include <gtest/gtest.h>

#include "zimovka/debug/DebugAccumulator.hpp"
#include "zimovka/debug/DebugStats.hpp"

using zimovka::DebugAccumulator;
using zimovka::DebugStats;

// ──────────────────────────────────────────────────────
// 初期状態
// ──────────────────────────────────────────────────────
/**
 * @brief DebugAccumulatorのインスタンス化
 * 
 */
TEST(DebugAccumulatorTest, InitialState){
    // 初期値は全てゼロ
    DebugAccumulator acc;
    EXPECT_EQ(acc.count, 0u);
    EXPECT_FLOAT_EQ(acc.sum_frame_ms,      0.0f);
    EXPECT_FLOAT_EQ(acc.sum_update_ms,     0.0f);
    EXPECT_FLOAT_EQ(acc.sum_render_ms,     0.0f);
    EXPECT_FLOAT_EQ(acc.sum_processing_ms, 0.0f);
    EXPECT_FLOAT_EQ(acc.max_frame_ms,      0.0f);
    EXPECT_EQ(acc.max_update_steps,    0u);
    EXPECT_EQ(acc.zero_update_frames,  0u);
    EXPECT_EQ(acc.multi_update_frames, 0u);
}

// ──────────────────────────────────────────────────────
// Accumulate
// ──────────────────────────────────────────────────────
/**
 * @brief Accumulate()を1回実行
 * 
 */
TEST(DebugAccumulatorTest, Accumulate_SingleFrame){
    DebugAccumulator acc;
    acc.Accumulate(16.0f, 1.0f, 2.0f, 4.0f, 1);
    EXPECT_EQ(acc.count, 1u);
    EXPECT_FLOAT_EQ(acc.sum_frame_ms,  16.0f);
    EXPECT_FLOAT_EQ(acc.sum_update_ms,  1.0f);
    EXPECT_FLOAT_EQ(acc.sum_render_ms,  2.0f);
    EXPECT_FLOAT_EQ(acc.max_frame_ms,  16.0f);
    EXPECT_EQ(acc.max_update_steps,    1u);
    EXPECT_EQ(acc.zero_update_frames,  0u);
    EXPECT_EQ(acc.multi_update_frames, 0u);
}

/**
 * @brief Accumulate()が呼び出されたがUpdateが実行されなかった
 * 
 */
TEST(DebugAccumulatorTest, Accumulate_ZeroUpdateStep){
    DebugAccumulator acc;
    acc.Accumulate(16.0f, 0.0f, 0.0f, 0.0f, 0);
    EXPECT_EQ(acc.max_update_steps,    0u);
    EXPECT_EQ(acc.zero_update_frames,  1u); // Update実行されなかったので1
    EXPECT_EQ(acc.multi_update_frames, 0u);
}

/**
 * @brief 1ゲームループ中に複数回Update()が実行された
 * 
 */
TEST(DebugAccumulatorTest, Accumulate_MultiUpdateStep){
    DebugAccumulator acc;
    acc.Accumulate(33.0f, 2.0f, 1.0f, 4.0f, 2);
    EXPECT_EQ(acc.max_update_steps,    2u); // 2回実行されたので2
    EXPECT_EQ(acc.multi_update_frames, 1u);
    EXPECT_EQ(acc.zero_update_frames,  0u);
}

/**
 * @brief Accumulate()を複数回実行したパターン
 * 
 */
TEST(DebugAccumulatorTest, Accumulate_MaxValues){
    DebugAccumulator acc;
    acc.Accumulate(10.0f, 3.0f, 1.0f, 2.0f, 1);
    acc.Accumulate(20.0f, 1.0f, 1.5f, 6.0f, 2);
    acc.Accumulate(15.0f, 2.0f, 2.0f, 4.0f, 1);
    // 上記の更新で出現した最大値が格納される
    EXPECT_FLOAT_EQ(acc.max_frame_ms,      20.0f);
    EXPECT_FLOAT_EQ(acc.max_update_ms,      3.0f);
    EXPECT_FLOAT_EQ(acc.max_render_ms,      2.0f);
    EXPECT_FLOAT_EQ(acc.max_processing_ms,  6.0f);
    EXPECT_EQ(acc.max_update_steps, 2u);
}

// ──────────────────────────────────────────────────────
// Reset
// ──────────────────────────────────────────────────────
/**
 * @brief Reset()のチェック
 * 
 */
TEST(DebugAccumulatorTest, Reset_ClearsAll){
    DebugAccumulator acc;
    acc.Accumulate(16.0f, 1.0f, 2.0f, 4.0f, 2);
    acc.Reset();
    EXPECT_EQ(acc.count,                  0u);
    EXPECT_FLOAT_EQ(acc.max_frame_ms,     0.0f);
    EXPECT_EQ(acc.max_update_steps,       0u);
    EXPECT_EQ(acc.zero_update_frames,     0u);
    EXPECT_EQ(acc.multi_update_frames,    0u);
}

// ──────────────────────────────────────────────────────
// FlushTo
// ──────────────────────────────────────────────────────
/**
 * @brief 複数回Accumulate()実行後のFlushTo()の実行
 * 
 */
TEST(DebugAccumulatorTest, FlushTo_ComputesAverage){
    DebugAccumulator acc;
    // 2回実行
    acc.Accumulate(10.0f, 1.0f, 2.0f, 4.0f, 1);
    acc.Accumulate(30.0f, 3.0f, 4.0f, 8.0f, 1);

    DebugStats stats;
    acc.FlushTo(stats);
    // 値は平均値となる
    EXPECT_FLOAT_EQ(stats.frame_ms,      20.0f); // (10+30)/2
    EXPECT_FLOAT_EQ(stats.update_ms,      2.0f); // (1+3)/2
    EXPECT_FLOAT_EQ(stats.render_ms,      3.0f); // (2+4)/2
    EXPECT_FLOAT_EQ(stats.processing_ms,  6.0f); // (4+8)/2
}

/**
 * @brief Accumulate()→FlushTo()での最大値のチェック
 * 
 */
TEST(DebugAccumulatorTest, FlushTo_WritesMax){
    DebugAccumulator acc;
    acc.Accumulate(10.0f, 1.0f, 1.0f, 2.0f, 1);
    acc.Accumulate(30.0f, 5.0f, 2.0f, 8.0f, 3);
    DebugStats stats;
    acc.FlushTo(stats);
    EXPECT_FLOAT_EQ(stats.max_frame_ms,     30.0f);
    EXPECT_FLOAT_EQ(stats.max_update_ms,     5.0f);
    EXPECT_FLOAT_EQ(stats.max_render_ms,     2.0f);
    EXPECT_FLOAT_EQ(stats.max_processing_ms, 8.0f);
    EXPECT_EQ(stats.max_update_steps, 3u);
}

/**
 * @brief Accumulate()→FlushTo()でデータの状態を確認 
 * 
 */
TEST(DebugAccumulatorTest, FlushTo_ResetsAccumulator){
    DebugAccumulator acc;
    acc.Accumulate(16.0f, 1.0f, 2.0f, 4.0f, 1);
    DebugStats stats;
    acc.FlushTo(stats);
    // Flush後はリセットされている
    EXPECT_EQ(acc.count, 0u);
    EXPECT_FLOAT_EQ(acc.sum_frame_ms, 0.0f);
    EXPECT_FLOAT_EQ(acc.max_frame_ms, 0.0f);
}

/**
 * @brief 
 * 
 */
TEST(DebugAccumulatorTest, FlushTo_EmptyDoesNotModifyOut){
    DebugAccumulator acc;
    DebugStats stats;
    stats.frame_ms = 99.0f; // sentinel
    acc.FlushTo(stats);
    // count == 0 なのでstatsは変化しない(Reset()が呼ばれていない)
    EXPECT_FLOAT_EQ(stats.frame_ms, 99.0f);
}
