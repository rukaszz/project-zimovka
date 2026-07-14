#include <gtest/gtest.h>

#include <cmath>

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
/**
 * @brief インスタンス化とその中心座標をテスト
 * 
 */
TEST(PlayerSystemTest, Initialize_Position){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const auto& p = ps.GetPlayer();
    EXPECT_FLOAT_EQ(p.position.x, W * 0.5f);
    EXPECT_FLOAT_EQ(p.position.y, H * 0.8f);
}

// ──────────────────────────────────────────────────────
// 移動
// ──────────────────────────────────────────────────────
/**
 * @brief 左方向への移動
 * 
 */
TEST(PlayerSystemTest, MoveLeft_DecreasesX){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const float x_before = ps.GetPlayer().position.x;

    InputState state;
    state.SetHeld(Action::MoveLeft, true);
    // 1フレーム分移動
    ps.Update((1.0f/60.0f), state);
    // 左移動なので前フレームより小さい
    EXPECT_LT(ps.GetPlayer().position.x, x_before);
}

/**
 * @brief 右方向へ移動
 * 
 */
TEST(PlayerSystemTest, MoveRight_IncreasesX){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const float x_before = ps.GetPlayer().position.x;

    InputState state;
    state.SetHeld(Action::MoveRight, true);
    ps.Update((1.0f/60.0f), state);
    // 右移動なので前フレームより大きい
    EXPECT_GT(ps.GetPlayer().position.x, x_before);
}

/**
 * @brief 上方向への移動(SDLはy軸の減少方向が上)
 * 
 */
TEST(PlayerSystemTest, MoveUp_DecreasesY){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const float y_before = ps.GetPlayer().position.y;

    InputState state;
    state.SetHeld(Action::MoveUp, true);
    ps.Update((1.0f/60.0f), state);
    // 上方向へ移動したので前フレームより小さい
    EXPECT_LT(ps.GetPlayer().position.y, y_before);
}

/**
 * @brief 下方向への移動
 * 
 */
TEST(PlayerSystemTest, MoveDown_IncreasesY){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const float y_before = ps.GetPlayer().position.y;

    InputState state;
    state.SetHeld(Action::MoveDown, true);
    ps.Update((1.0f/60.0f), state);
    // 下方向へ移動したので前フレームより大きい
    EXPECT_GT(ps.GetPlayer().position.y, y_before);
}

/**
 * @brief 斜め移動速度の正規化チェック
 *
 * 正規化されていれば「1フレームの斜め移動距離 = 1フレームの軸移動距離」となる
 * 正規化なしでは斜め移動が√2倍速になるため，変位ベクトルの大きさで比較する
 */
TEST(PlayerSystemTest, DiagonalMovementHasSameSpeed){
    constexpr float dt = 1.0f / 60.0f;
    const float start_x = W * 0.5f;
    const float start_y = H * 0.8f;

    // 1フレームの純粋な下移動距離
    PlayerSystem ps_axis;
    ps_axis.Initialize(W, H);
    // 下方向への移動入力
    InputState axis_input;
    axis_input.SetHeld(Action::MoveDown, true);
    ps_axis.Update(dt, axis_input);
    // 下方向移動時の移動距離
    const float dist_axis = ps_axis.GetPlayer().position.y - start_y;

    // 1フレームの斜め移動距離(右下)
    PlayerSystem ps_diag;
    ps_diag.Initialize(W, H);
    // 右下移動の入力
    InputState diag_input;
    diag_input.SetHeld(Action::MoveDown,  true);
    diag_input.SetHeld(Action::MoveRight, true);
    ps_diag.Update(dt, diag_input);
    // 移動距離を√(x^2+y^2)で算出
    const float dx = ps_diag.GetPlayer().position.x - start_x;
    const float dy = ps_diag.GetPlayer().position.y - start_y;
    const float dist_diag = std::sqrt(dx*dx + dy*dy);

    // 正規化されていれば移動距離(スピード * dt)が等しい
    EXPECT_NEAR(dist_axis, dist_diag, 1e-4f);
}

/**
 * @brief 相反する入力のチェック(左右)
 * 
 */
TEST(PlayerSystemTest, OppositeHorizontalInputsCancel){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const float x_before = ps.GetPlayer().position.x;

    InputState state;
    state.SetHeld(Action::MoveLeft,  true);
    state.SetHeld(Action::MoveRight, true);
    ps.Update((1.0f/60.0f), state);
    // 左と右方向の入力が発生したので動いていない
    EXPECT_NEAR(ps.GetPlayer().position.x, x_before, 1e-6f);
}

/**
 * @brief 相反する入力のチェック(上下)
 * 
 */
TEST(PlayerSystemTest, OppositeVerticalInputsCancel){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const float y_before = ps.GetPlayer().position.y;

    InputState state;
    state.SetHeld(Action::MoveUp,   true);
    state.SetHeld(Action::MoveDown, true);
    ps.Update((1.0f/60.0f), state);
    // 上と下方向の入力が発生したので動いていない
    EXPECT_NEAR(ps.GetPlayer().position.y, y_before, 1e-6f);
}

/**
 * @brief 入力がない状態でPlayerが移動しないかテスト
 * 
 */
TEST(PlayerSystemTest, NoInput_NoMove){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const float x_before = ps.GetPlayer().position.x;
    const float y_before = ps.GetPlayer().position.y;

    InputState state; // 何も押さない
    ps.Update((1.0f/60.0f), state);

    EXPECT_FLOAT_EQ(ps.GetPlayer().position.x, x_before);
    EXPECT_FLOAT_EQ(ps.GetPlayer().position.y, y_before);
}

/**
 * @brief Deltaが0の状態ではキャラクタが移動しない
 * 
 */
TEST(PlayerSystemTest, ZeroDeltaDoesNotMove){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const float x_before = ps.GetPlayer().position.x;
    const float y_before = ps.GetPlayer().position.y;

    InputState state;
    state.SetHeld(Action::MoveUp,   true);
    state.SetHeld(Action::MoveLeft, true);
    ps.Update(0.0f, state);

    EXPECT_FLOAT_EQ(ps.GetPlayer().position.x, x_before);
    EXPECT_FLOAT_EQ(ps.GetPlayer().position.y, y_before);
}

// ──────────────────────────────────────────────────────
// 低速移動(Slow)
// ──────────────────────────────────────────────────────
/**
 * @brief 通常速度→低速移動の移動速度を比較し低速になっていることを確認
 * 
 */
TEST(PlayerSystemTest, SlowMoveIsSlowerThanNormal){
    PlayerSystem ps;
    ps.Initialize(W, H);
    const float y_start = ps.GetPlayer().position.y;

    // 通常速度で上移動
    InputState normal;
    normal.SetHeld(Action::MoveUp, true);
    ps.Update((1.0f/60.0f), normal);
    const float dy_normal = y_start - ps.GetPlayer().position.y;

    // リセット
    ps.Initialize(W, H);

    // 低速で上移動
    InputState slow;
    slow.SetHeld(Action::MoveUp, true);
    slow.SetHeld(Action::Slow,   true);
    ps.Update((1.0f/60.0f), slow);
    const float dy_slow = y_start - ps.GetPlayer().position.y;

    EXPECT_GT(dy_normal, dy_slow); // 通常速度 > 低速
    EXPECT_GT(dy_slow, 0.0f);      // 低速でも移動はしている
}

// ──────────────────────────────────────────────────────
// 画面端クランプ
// ──────────────────────────────────────────────────────
/**
 * @brief 画面左端へのclamp処理
 * 
 */
TEST(PlayerSystemTest, ClampToScreen_Left){
    PlayerSystem ps;
    ps.Initialize(W, H);

    InputState state;
    // 300回左移動で更新して画面外へ出ていないか(画面端で止まっているか)チェック
    state.SetHeld(Action::MoveLeft, true);
    for(int i = 0; i < 300; ++i){
        ps.Update((1.0f/60.0f), state);
    }

    const auto& player   = ps.GetPlayer();
    const float player_x = player.position.x;
    const float half_w   = player.width * 0.5f;
    EXPECT_FLOAT_EQ(player_x, half_w);
}

/**
 * @brief 画面右端へのclamp処理
 * 
 */
TEST(PlayerSystemTest, ClampToScreen_Right){
    PlayerSystem ps;
    ps.Initialize(W, H);

    InputState state;
    state.SetHeld(Action::MoveRight, true);
    for(int i = 0; i < 300; ++i){
        ps.Update((1.0f/60.0f), state);
    }

    const auto& player   = ps.GetPlayer();
    const float player_x = player.position.x;
    const float half_w   = player.width * 0.5f;
    EXPECT_FLOAT_EQ(player_x, W - half_w);
}

/**
 * @brief 画面上端へのclamp処理
 * 
 */
TEST(PlayerSystemTest, ClampToScreen_Top){
    PlayerSystem ps;
    ps.Initialize(W, H);

    InputState state;
    state.SetHeld(Action::MoveUp, true);
    for(int i = 0; i < 300; ++i){
        ps.Update((1.0f/60.0f), state);
    }

    const auto& player   = ps.GetPlayer();
    const float player_y = player.position.y;
    const float half_h   = player.height * 0.5f;
    EXPECT_FLOAT_EQ(player_y, half_h);
}

/**
 * @brief 画面下端へのclamp処理
 * 
 */
TEST(PlayerSystemTest, ClampToScreen_Bottom){
    PlayerSystem ps;
    ps.Initialize(W, H);

    InputState state;
    state.SetHeld(Action::MoveDown, true);
    for(int i = 0; i < 300; ++i){
        ps.Update((1.0f/60.0f), state);
    }

    const auto& player   = ps.GetPlayer();
    const float player_y = player.position.y;
    const float half_h   = player.height * 0.5f;
    EXPECT_FLOAT_EQ(player_y, H - half_h);
}
