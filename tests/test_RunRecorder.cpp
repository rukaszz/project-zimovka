#include <gtest/gtest.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/replay/RunRecord.hpp"
#include "zimovka/replay/RunRecorder.hpp"

using zimovka::Action;
using zimovka::ActionBit;
using zimovka::InputState;
using zimovka::RunRecorder;

// ヘルパ関数
namespace{
/**
 * @brief held/pressed/releasedを全て同じビット列に設定したInputStateを作る
 *
 * @param bits
 * @return InputState
 */
InputState MakeInput(std::uint32_t bits){
    return InputState::FromBits(bits, bits, bits);
}

/**
 * @brief 単一Actionのみをheld/pressedに立てたInputStateを作る
 *
 * @param act
 * @return InputState
 */
InputState MakeActionInput(Action act){
    const std::uint32_t b = ActionBit(act);
    return InputState::FromBits(b, b, 0);
}

}   // namespace

// ──────────────────────────────────────────────────────
// 初期状態
// ──────────────────────────────────────────────────────
/**
 * @brief 生成直後は記録中でないことを確認
 *
 */
TEST(RunRecorderTest, InitialState_NotRecording){
    RunRecorder rr;
    EXPECT_FALSE(rr.IsRecording());
}

/**
 * @brief 生成直後のGetRecord()がフレーム0・seed 0であることを確認
 *
 */
TEST(RunRecorderTest, InitialState_EmptyRecord){
    RunRecorder rr;
    const auto& rec = rr.GetRecord();
    EXPECT_EQ(rec.random_seed, 0u);
    EXPECT_TRUE(rec.frames.empty());
}

/**
 * @brief 生成直後のRunRecordがtruncatedでないことを確認
 * 
 */
TEST(RunRecorderTest, InitialState_NotTruncated){
    RunRecorder rr;
    EXPECT_FALSE(rr.GetRecord().truncated);
}

// ──────────────────────────────────────────────────────
// Start
// ──────────────────────────────────────────────────────
/**
 * @brief Start()後に記録中になることを確認
 *
 */
TEST(RunRecorderTest, Start_SetsRecording){
    RunRecorder rr;
    rr.Start(42u);
    EXPECT_TRUE(rr.IsRecording());
}

/**
 * @brief Start()でseedが正しく設定されることを確認
 *
 */
TEST(RunRecorderTest, Start_SetsSeed){
    RunRecorder rr;
    rr.Start(12345u);
    EXPECT_EQ(rr.GetRecord().random_seed, 12345u);
}

/**
 * @brief Start()直後のframesは空であることを確認
 *
 */
TEST(RunRecorderTest, Start_FramesAreEmpty){
    RunRecorder rr;
    rr.Start(1u);
    EXPECT_TRUE(rr.GetRecord().frames.empty());
}

/**
 * @brief 2回Start()を呼んだとき，前の記録がリセットされていることを確認
 *
 */
TEST(RunRecorderTest, Start_ClearsPreviousRecord){
    RunRecorder rr;
    rr.Start(1u);
    rr.Record(MakeInput(0xFFu));
    // 再スタート
    rr.Start(2u);
    EXPECT_EQ(rr.GetRecord().random_seed, 2u);
    EXPECT_TRUE(rr.GetRecord().frames.empty());
}

// ──────────────────────────────────────────────────────
// Record
// ──────────────────────────────────────────────────────
/**
 * @brief Record()で入力フレームが積まれることを確認
 *
 */
TEST(RunRecorderTest, Record_AddsFrame){
    RunRecorder rr;
    rr.Start(0u);
    rr.Record(MakeInput(ActionBit(Action::MoveUp)));
    EXPECT_EQ(rr.GetRecord().frames.size(), 1u);
}

/**
 * @brief 記録中のRecord()がtrueを返すことを確認
 * 
 */
TEST(RunRecorderTest, Record_ReturnsTrue_WhenRecording){
    RunRecorder rr;
    rr.Start(0u);
    EXPECT_TRUE(rr.Record(MakeInput(0u)));
}

/**
 * @brief 記録中でない場合にRecord()がfalseを返し無視されることを確認
 *
 */
TEST(RunRecorderTest, Record_ReturnsFalse_WhenNotRecording){
    RunRecorder rr;
    // 全ビットが1の入力
    EXPECT_FALSE(rr.Record(MakeInput(0xFFu)));
    EXPECT_TRUE(rr.GetRecord().frames.empty());
}

/**
 * @brief 複数フレームを記録した場合のフレーム数を確認
 *
 */
TEST(RunRecorderTest, Record_MultipleFrames){
    RunRecorder rr;
    rr.Start(0u);
    // 3回記録
    rr.Record(MakeInput(0u));
    rr.Record(MakeInput(0u));
    rr.Record(MakeInput(0u));
    EXPECT_EQ(rr.GetRecord().frames.size(), 3u);
}

/**
 * @brief Record()でheld_bitsが正しく保存されることを確認
 *
 */
TEST(RunRecorderTest, Record_StoresHeldBits){
    RunRecorder rr;
    rr.Start(0u);
    // 左移動しながら発射する入力
    const std::uint32_t expected = ActionBit(Action::MoveLeft) | ActionBit(Action::Shoot);
    // キー押しっぱ(held)
    rr.Record(InputState::FromBits(expected, 0u, 0u));
    EXPECT_EQ(rr.GetRecord().frames[0].held_bits, expected);
}

// ──────────────────────────────────────────────────────
// 入力のフィルタリング
// ──────────────────────────────────────────────────────
/**
 * @brief Pauseのビットがフレームに記録されないことを確認
 *
 */
TEST(RunRecorderTest, Record_PauseFiltered){
    RunRecorder rr;
    rr.Start(0u);
    rr.Record(MakeActionInput(Action::Pause));
    const auto& f = rr.GetRecord().frames[0];
    EXPECT_EQ(f.held_bits,    0u);
    EXPECT_EQ(f.pressed_bits, 0u);
}

/**
 * @brief Quitのビットがフレームに記録されないことを確認
 *
 */
TEST(RunRecorderTest, Record_QuitFiltered){
    RunRecorder rr;
    rr.Start(0u);
    rr.Record(MakeActionInput(Action::Quit));
    const auto& f = rr.GetRecord().frames[0];
    EXPECT_EQ(f.held_bits,    0u);
    EXPECT_EQ(f.pressed_bits, 0u);
}

/**
 * @brief Pause/Quitを含む入力からゲームプレイ用Actionだけが残ることを確認
 *
 */
TEST(RunRecorderTest, Record_GameplayBitsKept_PauseQuitStripped){
    RunRecorder rr;
    rr.Start(0u);
    // ORで複数入力を連結
    const std::uint32_t all_bits =
        ActionBit(Action::MoveUp)   |
        ActionBit(Action::Shoot)    |
        ActionBit(Action::Pause)    |
        ActionBit(Action::Quit);
    // Pause/Quitは無視されているはず
    const std::uint32_t expected =
        ActionBit(Action::MoveUp) | ActionBit(Action::Shoot);
    // 記録
    rr.Record(InputState::FromBits(all_bits, all_bits, all_bits));
    const auto& f = rr.GetRecord().frames[0];
    // Pause/Quitは無い
    EXPECT_EQ(f.held_bits,     expected);
    EXPECT_EQ(f.pressed_bits,  expected);
    EXPECT_EQ(f.released_bits, expected);
}

// ──────────────────────────────────────────────────────
// Stop
// ──────────────────────────────────────────────────────
/**
 * @brief Stop()後に記録中でなくなることを確認
 *
 */
TEST(RunRecorderTest, Stop_ClearsRecording){
    RunRecorder rr;
    rr.Start(0u);
    rr.Stop();
    EXPECT_FALSE(rr.IsRecording());
}

/**
 * @brief Stop()後のRecord()が無視されることを確認
 *
 * Stop後はフレームが増えない
 */
TEST(RunRecorderTest, Stop_RecordIgnoredAfterStop){
    RunRecorder rr;
    rr.Start(0u);
    rr.Record(MakeInput(0u));
    rr.Stop();
    rr.Record(MakeInput(0u)); // 無視される
    EXPECT_EQ(rr.GetRecord().frames.size(), 1u);
}

/**
 * @brief Stop()後もGetRecord()でフレームが参照できることを確認
 *
 */
TEST(RunRecorderTest, Stop_RecordPreserved){
    RunRecorder rr;
    rr.Start(99u);
    rr.Record(MakeInput(ActionBit(Action::Shoot)));
    rr.Stop();
    EXPECT_EQ(rr.GetRecord().random_seed, 99u);
    EXPECT_EQ(rr.GetRecord().frames.size(), 1u);
}

/**
 * @brief 通常Stop()ではtruncatedが立たないことを確認
 * 
 */
TEST(RunRecorderTest, Stop_NotTruncated){
    RunRecorder rr;
    rr.Start(0u);
    rr.Record(MakeInput(0u));
    rr.Stop();
    EXPECT_FALSE(rr.GetRecord().truncated);
}

// ──────────────────────────────────────────────────────
// Clear
// ──────────────────────────────────────────────────────
/**
 * @brief Clear()後に記録中でなくなることを確認
 *
 */
TEST(RunRecorderTest, Clear_ClearsRecordingFlag){
    RunRecorder rr;
    rr.Start(0u);
    rr.Clear();
    EXPECT_FALSE(rr.IsRecording());
}

/**
 * @brief Clear()でseedとframesがリセットされることを確認
 *
 */
TEST(RunRecorderTest, Clear_ResetsRecord){
    RunRecorder rr;
    rr.Start(777u);
    rr.Record(MakeInput(0xFFu));
    rr.Clear();
    EXPECT_EQ(rr.GetRecord().random_seed, 0u);
    EXPECT_TRUE(rr.GetRecord().frames.empty());
}

/**
 * @brief Clear()後にStart()で正常に再開できることを確認
 *
 */
TEST(RunRecorderTest, Clear_AllowsRestart){
    RunRecorder rr;
    rr.Start(1u);
    rr.Clear();
    rr.Start(2u);
    EXPECT_TRUE(rr.IsRecording());
    EXPECT_EQ(rr.GetRecord().random_seed, 2u);
}

// ──────────────────────────────────────────────────────
// フレーム上限での自動停止
// ──────────────────────────────────────────────────────
/**
 * @brief MAX_RECORD_FRAMESに達したとき自動停止し，truncated=trueが立つことを確認
 *
 * 72,000フレーム = 20分 * 60fps に相当する上限
 */
TEST(RunRecorderTest, AutoStop_SetsRecordTruncated){
    RunRecorder rr;
    rr.Start(0u);
    const InputState empty_input;
    // 72,000フレーム記録して上限に到達させる
    constexpr std::size_t MAX_FRAMES = 72000u;
    for(std::size_t i = 0; i < MAX_FRAMES; ++i){
        rr.Record(empty_input);
    }
    EXPECT_FALSE(rr.IsRecording());           // 自動停止
    EXPECT_TRUE(rr.GetRecord().truncated);    // 途中終了フラグ
    EXPECT_EQ(rr.GetRecord().frames.size(), MAX_FRAMES);
}
