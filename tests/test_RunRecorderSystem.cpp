#include <gtest/gtest.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/replay/RunRecorder.hpp"
#include "zimovka/replay/RunRecorderSystem.hpp"

using zimovka::Action;
using zimovka::ActionBit;
using zimovka::InputState;
using zimovka::RunRecorderSystem;

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
TEST(RunRecorderSystemTest, InitialState_NotRecording){
    RunRecorderSystem rrs;
    EXPECT_FALSE(rrs.IsRecording());
}

/**
 * @brief 生成直後のGetRecord()がフレーム0・seed 0であることを確認
 * 
 */
TEST(RunRecorderSystemTest, InitialState_EmptyRecord){
    RunRecorderSystem rrs;
    const auto& rec = rrs.GetRecord();
    EXPECT_EQ(rec.random_seed, 0u);
    EXPECT_TRUE(rec.frames.empty());
}

// ──────────────────────────────────────────────────────
// Start
// ──────────────────────────────────────────────────────
/**
 * @brief Start()後に記録中になることを確認
 * 
 */
TEST(RunRecorderSystemTest, Start_SetsRecording){
    RunRecorderSystem rrs;
    rrs.Start(42u);
    EXPECT_TRUE(rrs.IsRecording());
}

/**
 * @brief Start()でseedが正しく設定されることを確認
 * 
 */
TEST(RunRecorderSystemTest, Start_SetsSeed){
    RunRecorderSystem rrs;
    rrs.Start(12345u);
    EXPECT_EQ(rrs.GetRecord().random_seed, 12345u);
}

/**
 * @brief Start()直後のframesは空であることを確認
 *
 */
TEST(RunRecorderSystemTest, Start_FramesAreEmpty){
    RunRecorderSystem rrs;
    rrs.Start(1u);
    EXPECT_TRUE(rrs.GetRecord().frames.empty());
}

/**
 * @brief 2回Start()を呼んだとき，前の記録がリセットされていることを確認
 * 
 */
TEST(RunRecorderSystemTest, Start_ClearsPreviousRecord){
    RunRecorderSystem rrs;
    rrs.Start(1u);
    rrs.Record(MakeInput(0xFFu));
    // 再スタート
    rrs.Start(2u);
    EXPECT_EQ(rrs.GetRecord().random_seed, 2u);
    EXPECT_TRUE(rrs.GetRecord().frames.empty());
}

// ──────────────────────────────────────────────────────
// Record
// ──────────────────────────────────────────────────────
/**
 * @brief Record()で入力フレームが積まれることを確認
 * 
 */
TEST(RunRecorderSystemTest, Record_AddsFrame){
    RunRecorderSystem rrs;
    rrs.Start(0u);
    rrs.Record(MakeInput(ActionBit(Action::MoveUp)));
    EXPECT_EQ(rrs.GetRecord().frames.size(), 1u);
}

/**
 * @brief 記録中でない場合にRecord()が無視されることを確認
 * 
 */
TEST(RunRecorderSystemTest, Record_WhenNotRecording_Ignored){
    RunRecorderSystem rrs;
    // 全ビットが1の入力
    rrs.Record(MakeInput(0xFFu));
    EXPECT_TRUE(rrs.GetRecord().frames.empty());
}

/**
 * @brief 複数フレームを記録した場合のフレーム数を確認
 * 
 */
TEST(RunRecorderSystemTest, Record_MultipleFrames){
    RunRecorderSystem rrs;
    rrs.Start(0u);
    // 3回記録
    rrs.Record(MakeInput(0u));
    rrs.Record(MakeInput(0u));
    rrs.Record(MakeInput(0u));
    EXPECT_EQ(rrs.GetRecord().frames.size(), 3u);
}

/**
 * @brief Record()でheld_bitsが正しく保存されることを確認
 * 
 */
TEST(RunRecorderSystemTest, Record_StoresHeldBits){
    RunRecorderSystem rrs;
    rrs.Start(0u);
    // 左移動しながら発射する入力
    const std::uint32_t expected = ActionBit(Action::MoveLeft) | ActionBit(Action::Shoot);
    // キー押しっぱ(held)
    rrs.Record(InputState::FromBits(expected, 0u, 0u));
    EXPECT_EQ(rrs.GetRecord().frames[0].held_bits, expected);
}

// ──────────────────────────────────────────────────────
// Pause / Quit のフィルタリング
// ──────────────────────────────────────────────────────
/**
 * @brief Pauseのビットがフレームに記録されないことを確認
 * 
 */
TEST(RunRecorderSystemTest, Record_PauseFiltered){
    RunRecorderSystem rrs;
    rrs.Start(0u);
    rrs.Record(MakeActionInput(Action::Pause));
    const auto& f = rrs.GetRecord().frames[0];
    EXPECT_EQ(f.held_bits,    0u);
    EXPECT_EQ(f.pressed_bits, 0u);
}

/**
 * @brief Quitのビットがフレームに記録されないことを確認
 * 
 */
TEST(RunRecorderSystemTest, Record_QuitFiltered){
    RunRecorderSystem rrs;
    rrs.Start(0u);
    rrs.Record(MakeActionInput(Action::Quit));
    const auto& f = rrs.GetRecord().frames[0];
    EXPECT_EQ(f.held_bits,    0u);
    EXPECT_EQ(f.pressed_bits, 0u);
}

/**
 * @brief Pause/Quitを含む入力からゲームプレイ用Actionだけが残ることを確認
 * 
 */
TEST(RunRecorderSystemTest, Record_GameplayBitsKept_PauseQuitStripped){
    RunRecorderSystem rrs;
    rrs.Start(0u);
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
    rrs.Record(InputState::FromBits(all_bits, all_bits, all_bits));
    const auto& f = rrs.GetRecord().frames[0];
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
TEST(RunRecorderSystemTest, Stop_ClearsRecording){
    RunRecorderSystem rrs;
    rrs.Start(0u);
    rrs.Stop();
    EXPECT_FALSE(rrs.IsRecording());
}

/**
 * @brief Stop()後のRecord()が無視されることを確認
 *
 * Stop後はフレームが増えない
 */
TEST(RunRecorderSystemTest, Stop_RecordIgnoredAfterStop){
    RunRecorderSystem rrs;
    rrs.Start(0u);
    rrs.Record(MakeInput(0u));
    rrs.Stop();
    rrs.Record(MakeInput(0u)); // 無視される
    EXPECT_EQ(rrs.GetRecord().frames.size(), 1u);
}

/**
 * @brief Stop()後もGetRecord()でフレームが参照できることを確認
 * 
 */
TEST(RunRecorderSystemTest, Stop_RecordPreserved){
    RunRecorderSystem rrs;
    rrs.Start(99u);
    rrs.Record(MakeInput(ActionBit(Action::Shoot)));
    rrs.Stop();
    EXPECT_EQ(rrs.GetRecord().random_seed, 99u);
    EXPECT_EQ(rrs.GetRecord().frames.size(), 1u);
}

// ──────────────────────────────────────────────────────
// Clear
// ──────────────────────────────────────────────────────
/**
 * @brief Clear()後に記録中でなくなることを確認
 * 
 */
TEST(RunRecorderSystemTest, Clear_ClearsRecordingFlag){
    RunRecorderSystem rrs;
    rrs.Start(0u);
    rrs.Clear();
    EXPECT_FALSE(rrs.IsRecording());
}

/**
 * @brief Clear()でseedとframesがリセットされることを確認
 * 
 */
TEST(RunRecorderSystemTest, Clear_ResetsRecord){
    RunRecorderSystem rrs;
    rrs.Start(777u);
    rrs.Record(MakeInput(0xFFu));
    rrs.Clear();
    EXPECT_EQ(rrs.GetRecord().random_seed, 0u);
    EXPECT_TRUE(rrs.GetRecord().frames.empty());
}

/**
 * @brief Clear()後にStart()で正常に再開できることを確認
 * 
 */
TEST(RunRecorderSystemTest, Clear_AllowsRestart){
    RunRecorderSystem rrs;
    rrs.Start(1u);
    rrs.Clear();
    rrs.Start(2u);
    EXPECT_TRUE(rrs.IsRecording());
    EXPECT_EQ(rrs.GetRecord().random_seed, 2u);
}
