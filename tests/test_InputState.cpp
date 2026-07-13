#include <gtest/gtest.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"

using zimovka::Action;
using zimovka::InputState;

// ──────────────────────────────────────────────────────
// InputState 単体テスト
// ──────────────────────────────────────────────────────

/**
 * @brief デフォルト状態テスト
 * 
 */
TEST(InputStateTest, DefaultAllFalse){
    InputState state;
    EXPECT_FALSE(state.IsHeld(Action::MoveUp));
    EXPECT_FALSE(state.IsHeld(Action::MoveDown));
    EXPECT_FALSE(state.IsPressed(Action::MoveUp));
    EXPECT_FALSE(state.IsReleased(Action::MoveUp));
    EXPECT_FALSE(state.IsHeld(Action::Slow));
    EXPECT_FALSE(state.IsPressed(Action::Slow));
    EXPECT_FALSE(state.IsReleased(Action::Slow));
}

/**
 * @brief キーの連続押下(held)をチェック
 * 
 */
TEST(InputStateTest, SetHeld_True){
    InputState state;
    // 左移動を押下
    state.SetHeld(Action::MoveLeft, true);
    EXPECT_TRUE(state.IsHeld(Action::MoveLeft));
    // 他のボタン状態には影響しない
    EXPECT_FALSE(state.IsPressed(Action::MoveLeft));
    EXPECT_FALSE(state.IsReleased(Action::MoveLeft));
    // 他のActionには影響しない
    EXPECT_FALSE(state.IsHeld(Action::MoveRight));
}

/**
 * @brief 押下状態から脱したパターンをチェック
 * 
 */
TEST(InputStateTest, SetHeld_False){
    InputState state;
    state.SetHeld(Action::MoveLeft, true);
    state.SetHeld(Action::MoveLeft, false);
    // ボタンは離れているのでfalse
    EXPECT_FALSE(state.IsHeld(Action::MoveLeft));
    // 他のActionには影響しない
    EXPECT_FALSE(state.IsHeld(Action::MoveRight));
}

/**
 * @brief Pressed(押下された瞬間)をテスト
 * 
 */
TEST(InputStateTest, SetPressed){
    InputState state;
    // Pressedのみセット
    state.SetPressed(Action::Shoot);
    EXPECT_TRUE(state.IsPressed(Action::Shoot));
    // held/Releasedはfalseのまま
    EXPECT_FALSE(state.IsHeld(Action::Shoot));   // pressed ≠ held であることを確認
    EXPECT_FALSE(state.IsReleased(Action::Shoot)); 
}

/**
 * @brief Released(ボタンが離れた状態)をテスト
 * 
 */
TEST(InputStateTest, SetReleased){
    InputState state;
    state.SetReleased(Action::Bomb);
    EXPECT_TRUE(state.IsReleased(Action::Bomb));
    EXPECT_FALSE(state.IsHeld(Action::Bomb));
    EXPECT_FALSE(state.IsPressed(Action::Bomb));
}

/**
 * @brief 複数のActionを実施したパターン
 * 
 */
TEST(InputStateTest, MultipleActions_Independent){
    InputState state;
    // MoveUp/MoveLeftを実施
    state.SetHeld(Action::MoveUp,   true);
    state.SetHeld(Action::MoveLeft, true);
    EXPECT_TRUE(state.IsHeld(Action::MoveUp));
    EXPECT_TRUE(state.IsHeld(Action::MoveLeft));
    // 関係ないボタンはfalse
    EXPECT_FALSE(state.IsHeld(Action::MoveDown));
    EXPECT_FALSE(state.IsHeld(Action::MoveRight));
}

/**
 * @brief 一時的な状態(Pressed/Released)のリセットをテスト
 * 
 */
TEST(InputStateTest, ClearTransient_KeepsHeld){
    InputState state;
    // Held/Pressed/Releasedをセット
    state.SetHeld(Action::MoveUp, true);
    state.SetPressed(Action::MoveUp);
    state.SetReleased(Action::MoveDown);
    // Pressed/Releasedがリセットされるはず
    state.ClearTransient();
    EXPECT_TRUE(state.IsHeld(Action::MoveUp));          // heldは保持される
    EXPECT_FALSE(state.IsPressed(Action::MoveUp));      // pressedはクリア
    EXPECT_FALSE(state.IsReleased(Action::MoveDown));   // releasedはクリア
}

/**
 * @brief 全状態リセット関数をテスト
 * 
 */
TEST(InputStateTest, ClearAll){
    InputState state;
    // Held/Pressed/Releasedをセット
    state.SetHeld(Action::MoveUp,   true);
    state.SetHeld(Action::MoveLeft, true);
    state.SetPressed(Action::Shoot);
    state.SetReleased(Action::Slow);
    // 全て状態をリセット
    state.ClearAll();
    // 初期状態になっている
    EXPECT_FALSE(state.IsHeld(Action::MoveUp));
    EXPECT_FALSE(state.IsHeld(Action::MoveLeft));
    EXPECT_FALSE(state.IsPressed(Action::Shoot));
    EXPECT_FALSE(state.IsReleased(Action::Slow));
}

/**
 * @brief bit maskのテスト
 * 
 */
TEST(InputStateTest, FromBits){
    using zimovka::ActionBit;
    // MoveUp/MoveRightのビッドを立てる※1, 4番目が立つ
    const std::uint32_t held = ActionBit(Action::MoveUp) | ActionBit(Action::MoveRight);
    // heldのみFromBitsへ渡す
    auto state = InputState::FromBits(held, 0u, 0u);
    // 対応するInputStateがTRUEになる
    EXPECT_TRUE(state.IsHeld(Action::MoveUp));
    EXPECT_TRUE(state.IsHeld(Action::MoveRight));
    // Held以外に影響はない
    EXPECT_FALSE(state.IsPressed(Action::MoveUp));
    EXPECT_FALSE(state.IsReleased(Action::MoveUp));
    // 関係ないボタンも影響がない
    EXPECT_FALSE(state.IsHeld(Action::MoveDown));
    EXPECT_FALSE(state.IsHeld(Action::MoveLeft));
}

/**
 * @brief アクセサのテスト
 * 
 */
TEST(InputStateTest, GetBitsConsistency){
    using zimovka::ActionBit;
    InputState state;
    // 各種の状態をセット
    state.SetHeld(Action::Slow, true);
    state.SetPressed(Action::Shoot);
    state.SetReleased(Action::Pause);
    // 現在のInputStateの状態 と ActionBitで対応するビットが等しい
    EXPECT_EQ(state.GetHeldBits(),     ActionBit(Action::Slow));
    EXPECT_EQ(state.GetPressedBits(),  ActionBit(Action::Shoot));
    EXPECT_EQ(state.GetReleasedBits(), ActionBit(Action::Pause));
}
