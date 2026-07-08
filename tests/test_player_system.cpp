#include <gtest/gtest.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/systems/player/PlayerSystem.hpp"

using zimovka::Action;
using zimovka::InputState;
using zimovka::PlayerSystem;

static constexpr float W = 960.0f;
static constexpr float H = 720.0f;

// ──────────────────────────────────────────────────────
// 初期化
// ──────────────────────────────────────────────────────
TEST(PlayerSystemTest, Initialize_Position) {
    PlayerSystem sys;
    sys.Initialize(W, H);
    const auto& p = sys.GetPlayer();
    EXPECT_FLOAT_EQ(p.position.x, W * 0.5f);
    EXPECT_FLOAT_EQ(p.position.y, H * 0.8f);
}

// ──────────────────────────────────────────────────────
// 移動
// ──────────────────────────────────────────────────────
TEST(PlayerSystemTest, MoveUp_DecreasesY) {
    PlayerSystem sys;
    sys.Initialize(W, H);
    const float y_before = sys.GetPlayer().position.y;

    InputState state;
    state.SetHeld(Action::MoveUp, true);
    sys.Update(1.0f / 60.0f, state);

    EXPECT_LT(sys.GetPlayer().position.y, y_before);
}

TEST(PlayerSystemTest, MoveDown_IncreasesY) {
    PlayerSystem sys;
    sys.Initialize(W, H);
    const float y_before = sys.GetPlayer().position.y;

    InputState state;
    state.SetHeld(Action::MoveDown, true);
    sys.Update(1.0f / 60.0f, state);

    EXPECT_GT(sys.GetPlayer().position.y, y_before);
}

TEST(PlayerSystemTest, MoveLeft_DecreasesX) {
    PlayerSystem sys;
    sys.Initialize(W, H);
    const float x_before = sys.GetPlayer().position.x;

    InputState state;
    state.SetHeld(Action::MoveLeft, true);
    sys.Update(1.0f / 60.0f, state);

    EXPECT_LT(sys.GetPlayer().position.x, x_before);
}

TEST(PlayerSystemTest, MoveRight_IncreasesX) {
    PlayerSystem sys;
    sys.Initialize(W, H);
    const float x_before = sys.GetPlayer().position.x;

    InputState state;
    state.SetHeld(Action::MoveRight, true);
    sys.Update(1.0f / 60.0f, state);

    EXPECT_GT(sys.GetPlayer().position.x, x_before);
}

TEST(PlayerSystemTest, NoInput_NoMove) {
    PlayerSystem sys;
    sys.Initialize(W, H);
    const float x_before = sys.GetPlayer().position.x;
    const float y_before = sys.GetPlayer().position.y;

    InputState state; // 何も押さない
    sys.Update(1.0f / 60.0f, state);

    EXPECT_FLOAT_EQ(sys.GetPlayer().position.x, x_before);
    EXPECT_FLOAT_EQ(sys.GetPlayer().position.y, y_before);
}

// ──────────────────────────────────────────────────────
// 低速移動(Slow)
// ──────────────────────────────────────────────────────
TEST(PlayerSystemTest, SlowMoveIsSlowerThanNormal) {
    PlayerSystem sys;
    sys.Initialize(W, H);
    const float y_start = sys.GetPlayer().position.y;

    // 通常速度で上移動
    InputState normal;
    normal.SetHeld(Action::MoveUp, true);
    sys.Update(1.0f / 60.0f, normal);
    const float dy_normal = y_start - sys.GetPlayer().position.y;

    // リセット
    sys.Initialize(W, H);

    // 低速で上移動
    InputState slow;
    slow.SetHeld(Action::MoveUp, true);
    slow.SetHeld(Action::Slow,   true);
    sys.Update(1.0f / 60.0f, slow);
    const float dy_slow = y_start - sys.GetPlayer().position.y;

    EXPECT_GT(dy_normal, dy_slow); // 通常速度 > 低速
    EXPECT_GT(dy_slow, 0.0f);      // 低速でも移動している
}

// ──────────────────────────────────────────────────────
// 画面端クランプ
// ──────────────────────────────────────────────────────
TEST(PlayerSystemTest, ClampToScreen_Left) {
    PlayerSystem sys;
    sys.Initialize(W, H);

    InputState state;
    state.SetHeld(Action::MoveLeft, true);
    for(int i = 0; i < 300; ++i){
        sys.Update(1.0f / 60.0f, state);
    }
    EXPECT_GE(sys.GetPlayer().position.x, 0.0f);
}

TEST(PlayerSystemTest, ClampToScreen_Right) {
    PlayerSystem sys;
    sys.Initialize(W, H);

    InputState state;
    state.SetHeld(Action::MoveRight, true);
    for(int i = 0; i < 300; ++i){
        sys.Update(1.0f / 60.0f, state);
    }
    EXPECT_LE(sys.GetPlayer().position.x, W);
}

TEST(PlayerSystemTest, ClampToScreen_Top) {
    PlayerSystem sys;
    sys.Initialize(W, H);

    InputState state;
    state.SetHeld(Action::MoveUp, true);
    for(int i = 0; i < 300; ++i){
        sys.Update(1.0f / 60.0f, state);
    }
    EXPECT_GE(sys.GetPlayer().position.y, 0.0f);
}

TEST(PlayerSystemTest, ClampToScreen_Bottom) {
    PlayerSystem sys;
    sys.Initialize(W, H);

    InputState state;
    state.SetHeld(Action::MoveDown, true);
    for(int i = 0; i < 300; ++i){
        sys.Update(1.0f / 60.0f, state);
    }
    EXPECT_LE(sys.GetPlayer().position.y, H);
}
