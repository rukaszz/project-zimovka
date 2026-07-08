#include <gtest/gtest.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"

using zimovka::Action;
using zimovka::InputState;

// ──────────────────────────────────────────────────────
// InputState 単体テスト
// ──────────────────────────────────────────────────────

TEST(InputStateTest, DefaultAllFalse) {
    InputState state;
    EXPECT_FALSE(state.IsHeld(Action::MoveUp));
    EXPECT_FALSE(state.IsHeld(Action::MoveDown));
    EXPECT_FALSE(state.IsPressed(Action::MoveUp));
    EXPECT_FALSE(state.IsReleased(Action::MoveUp));
}

TEST(InputStateTest, SetHeld_True) {
    InputState state;
    state.SetHeld(Action::MoveLeft, true);
    EXPECT_TRUE(state.IsHeld(Action::MoveLeft));
    // 他のActionには影響しない
    EXPECT_FALSE(state.IsHeld(Action::MoveRight));
}

TEST(InputStateTest, SetHeld_False) {
    InputState state;
    state.SetHeld(Action::MoveLeft, true);
    state.SetHeld(Action::MoveLeft, false);
    EXPECT_FALSE(state.IsHeld(Action::MoveLeft));
}

TEST(InputStateTest, SetPressed) {
    InputState state;
    state.SetPressed(Action::Shoot);
    EXPECT_TRUE(state.IsPressed(Action::Shoot));
    EXPECT_FALSE(state.IsHeld(Action::Shoot));   // pressed ≠ held
}

TEST(InputStateTest, SetReleased) {
    InputState state;
    state.SetReleased(Action::Bomb);
    EXPECT_TRUE(state.IsReleased(Action::Bomb));
    EXPECT_FALSE(state.IsHeld(Action::Bomb));
}

TEST(InputStateTest, MultipleActions_Independent) {
    InputState state;
    state.SetHeld(Action::MoveUp,   true);
    state.SetHeld(Action::MoveLeft, true);
    EXPECT_TRUE(state.IsHeld(Action::MoveUp));
    EXPECT_TRUE(state.IsHeld(Action::MoveLeft));
    EXPECT_FALSE(state.IsHeld(Action::MoveDown));
    EXPECT_FALSE(state.IsHeld(Action::MoveRight));
}

TEST(InputStateTest, ClearTransient_KeepsHeld) {
    InputState state;
    state.SetHeld(Action::MoveUp, true);
    state.SetPressed(Action::MoveUp);
    state.SetReleased(Action::MoveDown);
    state.ClearTransient();
    EXPECT_TRUE(state.IsHeld(Action::MoveUp));      // held は保持される
    EXPECT_FALSE(state.IsPressed(Action::MoveUp));  // pressed はクリア
    EXPECT_FALSE(state.IsReleased(Action::MoveDown)); // released はクリア
}

TEST(InputStateTest, ClearAll) {
    InputState state;
    state.SetHeld(Action::MoveUp,   true);
    state.SetHeld(Action::MoveLeft, true);
    state.SetPressed(Action::Shoot);
    state.ClearAll();
    EXPECT_FALSE(state.IsHeld(Action::MoveUp));
    EXPECT_FALSE(state.IsHeld(Action::MoveLeft));
    EXPECT_FALSE(state.IsPressed(Action::Shoot));
}

TEST(InputStateTest, FromBits) {
    using zimovka::ActionBit;
    const std::uint32_t held = ActionBit(Action::MoveUp) | ActionBit(Action::MoveRight);
    auto state = InputState::FromBits(held, 0u, 0u);
    EXPECT_TRUE(state.IsHeld(Action::MoveUp));
    EXPECT_TRUE(state.IsHeld(Action::MoveRight));
    EXPECT_FALSE(state.IsHeld(Action::MoveDown));
    EXPECT_FALSE(state.IsHeld(Action::MoveLeft));
}

TEST(InputStateTest, GetBitsConsistency) {
    using zimovka::ActionBit;
    InputState state;
    state.SetHeld(Action::Slow, true);
    state.SetPressed(Action::Shoot);
    state.SetReleased(Action::Pause);

    EXPECT_EQ(state.GetHeldBits(),     ActionBit(Action::Slow));
    EXPECT_EQ(state.GetPressedBits(),  ActionBit(Action::Shoot));
    EXPECT_EQ(state.GetReleasedBits(), ActionBit(Action::Pause));
}
