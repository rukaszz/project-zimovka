#include <gtest/gtest.h>

#include <SDL2/SDL.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputSystem.hpp"

using zimovka::Action;
using zimovka::InputSystem;

// 注意：ヘルパ関数呼び出しにSDL_Initは不要→SDL_Eventは単なるunion構造体なのでゼロ初期化で生成できるため
namespace{
/**
 * @brief キーイベントを生成するテストヘルパ関数
 * 
 * @param type 
 * @param sc 
 * @param repeat 
 * @return SDL_Event 
 */
SDL_Event MakeKeyEvent(Uint32 type, SDL_Scancode sc, Uint8 repeat = 0){
    SDL_Event e{};
    e.type                = type;
    e.key.type            = type;
    e.key.keysym.scancode = sc;
    e.key.repeat          = repeat;
    return e;
}

/**
 * @brief キー押下イベント
 * 
 * @param sc 
 * @param repeat 
 * @return SDL_Event 
 */
SDL_Event MakeKeyDown(SDL_Scancode sc, bool repeat = false){
    return MakeKeyEvent(SDL_KEYDOWN, sc, repeat ? 1u : 0u);
}

/**
 * @brief キーを離した際のイベント
 * 
 * @param sc 
 * @return SDL_Event 
 */
SDL_Event MakeKeyUp(SDL_Scancode sc){
    return MakeKeyEvent(SDL_KEYUP, sc);
}

/**
 * @brief ウィンドウのフォーカスが離れたケースのテスト用
 * 
 * @return SDL_Event 
 */
SDL_Event MakeFocusLost(){
    SDL_Event e{};
    e.type         = SDL_WINDOWEVENT;
    e.window.type  = SDL_WINDOWEVENT;
    e.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
    return e;
}

}   // namespace

// ──────────────────────────────────────────────────────
// 基本的なpress / release
// ──────────────────────────────────────────────────────
/**
 * @brief KEYDOWNでIsPressed()がtrueになることを確認
 * 
 */
TEST(InputSystemTest, KeyDown_SetsPressed){
    InputSystem sys;
    // Wキー押下
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    // Action::MoveUpが有効化
    EXPECT_TRUE(sys.IsPressed(Action::MoveUp));
}

/**
 * @brief KEYDOWNでIsHeld()がtrueになることを確認
 * 
 */
TEST(InputSystemTest, KeyDown_SetsHeld){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    EXPECT_TRUE(sys.IsHeld(Action::MoveUp));
}

/**
 * @brief KEYUPでIsReleased()がtrueになることを確認
 * 
 */
TEST(InputSystemTest, KeyUp_SetsReleased){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_W));
    EXPECT_TRUE(sys.IsReleased(Action::MoveUp));
}

/**
 * @brief KEYUPでIsHeld()がfalseになることを確認
 * 
 */
TEST(InputSystemTest, KeyUp_ClearsHeld){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_W));
    EXPECT_FALSE(sys.IsHeld(Action::MoveUp));
}

/**
 * @brief KEYDOWNなしにKEYUPが来ても状態を変化させないことを確認
 * 
 * KEYDOWN→KEYUPの遷移を正しく認識しカウントが壊れないかチェック
 */
TEST(InputSystemTest, KeyUp_WithoutPriorKeyDown_DoesNothing){
    // physical_key_held_[]のガードで弾かれるので状態変化はない
    InputSystem sys;
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_W));
    EXPECT_FALSE(sys.IsReleased(Action::MoveUp));
    EXPECT_FALSE(sys.IsHeld(Action::MoveUp));
}

// ──────────────────────────────────────────────────────
// 物理キーの重複KEYDOWN
// ──────────────────────────────────────────────────────
/**
 * @brief 同一物理キーのrepeat=false な非リピートKEYDOWNが連続して来ても1回分しか処理されないことを確認
 *
 * physical_key_held_[]が同一スキャンコードの重複入力を弾くことをチェック
 */
TEST(InputSystemTest, DuplicateKeyDown_SamePhysicalKey_Ignored){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W, false)); // 1回目: 正常処理
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W, false)); // 2回目: 無視される
    // KEYUPで正常にReleasedが立ちHeldが解除される(2回分カウントされていれば解除されない)
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_W));
    EXPECT_TRUE(sys.IsReleased(Action::MoveUp));
    EXPECT_FALSE(sys.IsHeld(Action::MoveUp));
}

/**
 * @brief OSレベルのキーリピート(repeat=true)が無視されることを確認
 * 
 */
TEST(InputSystemTest, KeyRepeat_Ignored){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W, false)); // 最初の押下
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W, true));  // OSリピート → 無視
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W, true));  // OSリピート → 無視
    // Held状態が維持されている
    EXPECT_TRUE(sys.IsHeld(Action::MoveUp));
}

// ──────────────────────────────────────────────────────
// 同一Actionに対応する複数キー (hold_counts_ 参照カウント)
// ──────────────────────────────────────────────────────
/**
 * @brief W + ↑同時押しでMoveUpが継続されることを確認
 * 
 */
TEST(InputSystemTest, MultipleKeys_SameAction_BothDown_Held){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_UP));
    EXPECT_TRUE(sys.IsHeld(Action::MoveUp));
}

/**
 * @brief W + ↑でWを離してもMoveUpが維持されることを確認
 *
 * hold_counts_[MoveUp] == 2→Wを離してcount == 1, まだ↑が押されているのでheld維持
 */
TEST(InputSystemTest, MultipleKeys_SameAction_ReleaseOne_StillHeld){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_UP));
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_W));     // Wを離す
    EXPECT_TRUE(sys.IsHeld(Action::MoveUp));        // ↑がまだ押されている
    EXPECT_FALSE(sys.IsReleased(Action::MoveUp));   // Releasedにはならない
}

/**
 * @brief W + ↑で両方離した後にMoveUpがReleasedになることを確認
 *
 * hold_counts_[MoveUp] == 0 → SetHeld(false), SetReleased
 */
TEST(InputSystemTest, MultipleKeys_SameAction_ReleaseBoth_Released){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_UP));
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_W));
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_UP));
    EXPECT_FALSE(sys.IsHeld(Action::MoveUp));
    EXPECT_TRUE(sys.IsReleased(Action::MoveUp));
}

/**
 * @brief LSHIFT + RSHIFT でLShiftを離してもSlow維持されることを確認
 * 
 */
TEST(InputSystemTest, Slow_BothShifts_ReleaseOne_StillHeld){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_LSHIFT));
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_RSHIFT));
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_LSHIFT));
    EXPECT_TRUE(sys.IsHeld(Action::Slow));           // RSHIFTがまだ押されている
    EXPECT_FALSE(sys.IsReleased(Action::Slow));
}

// ──────────────────────────────────────────────────────
// フォーカス喪失
// ──────────────────────────────────────────────────────
/**
 * @brief フォーカス喪失でHeld状態がクリアされることを確認
 * 
 */
TEST(InputSystemTest, FocusLost_ClearsHeld){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    EXPECT_TRUE(sys.IsHeld(Action::MoveUp));
    // ウィンドウのフォーカスロスト
    sys.HandleEvent(MakeFocusLost());
    EXPECT_FALSE(sys.IsHeld(Action::MoveUp));
}

/**
 * @brief フォーカス喪失で複数のHeld状態がまとめてクリアされることを確認
 * 
 */
TEST(InputSystemTest, FocusLost_ClearsAllHeld){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_LSHIFT));
    sys.HandleEvent(MakeFocusLost());
    // ウィンドウフォーカスロストで状態が非活性に戻る
    EXPECT_FALSE(sys.IsHeld(Action::MoveUp));
    EXPECT_FALSE(sys.IsHeld(Action::Slow));
}

/**
 * @brief フォーカス喪失後の遅延KEYUPが無視されることを確認
 *
 * フォーカス喪失時にphysical_key_held_[]がfill(false)によってfalseになるため，
 * 後続のKEYUPはphysical_key_held_[idx]==falseとして弾かれる
 */
TEST(InputSystemTest, FocusLost_DelayedKeyUp_DoesNotSetReleased){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    sys.HandleEvent(MakeFocusLost());
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_W)); // フォーカス喪失後に遅延してKEYUPが走る
    EXPECT_FALSE(sys.IsReleased(Action::MoveUp)); // 無視される
}

/**
 * @brief フォーカス再取得後の入力が正常に処理されることを確認
 * 
 */
TEST(InputSystemTest, FocusLost_ThenKeyDown_WorksNormally){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    sys.HandleEvent(MakeFocusLost());
    // フォーカス再取得後
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    EXPECT_TRUE(sys.IsHeld(Action::MoveUp));
    EXPECT_TRUE(sys.IsPressed(Action::MoveUp));
}

// ──────────────────────────────────────────────────────
// ConsumeStateForTick()
// ──────────────────────────────────────────────────────
/**
 * @brief ConsumeStateForTick()がPressed+Heldのスナップショットを返すことを確認
 * 
 */
TEST(InputSystemTest, ConsumeStateForTick_ReturnsPressedAndHeld){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    // 入力スナップショット取得
    const auto tick = sys.ConsumeStateForTick();
    EXPECT_TRUE(tick.IsPressed(Action::MoveUp));
    EXPECT_TRUE(tick.IsHeld(Action::MoveUp));
}

/**
 * @brief 2回目のConsumeではPressedがクリアされHeldが残ることを確認
 * 
 */
TEST(InputSystemTest, ConsumeStateForTick_SecondTick_PressedCleared){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    (void)sys.ConsumeStateForTick();                  // tick1：pressed + held
    const auto tick2 = sys.ConsumeStateForTick();     // tick2：heldのみになる
    EXPECT_FALSE(tick2.IsPressed(Action::MoveUp));
    EXPECT_TRUE(tick2.IsHeld(Action::MoveUp));
}

/**
 * @brief Heldは複数Tickをまたいで維持されることを確認
 * 
 */
TEST(InputSystemTest, ConsumeStateForTick_HeldPersistsAcrossTicks){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_W));
    (void)sys.ConsumeStateForTick(); // tick1
    (void)sys.ConsumeStateForTick(); // tick2
    const auto tick3 = sys.ConsumeStateForTick();
    // 何Tick経ってもheldは維持されている
    EXPECT_TRUE(tick3.IsHeld(Action::MoveUp));
    EXPECT_FALSE(tick3.IsPressed(Action::MoveUp));
}

/**
 * @brief ReleasedはConsumeにより1回のtickにしか現れないことを確認
 * 
 */
TEST(InputSystemTest, ConsumeStateForTick_Released_AppearsOnlyOnce){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_Z));
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_Z));
    // releasedを受け取るtick
    const auto tick1 = sys.ConsumeStateForTick();
    EXPECT_TRUE(tick1.IsReleased(Action::Shoot));   // tick1でreleased
    // tick2では消えている
    const auto tick2 = sys.ConsumeStateForTick();
    EXPECT_FALSE(tick2.IsReleased(Action::Shoot));  
}

// ──────────────────────────────────────────────────────
// press-release (同一フレーム内での即押し・即離し)
// ──────────────────────────────────────────────────────
/**
 * @brief 同一フレーム内でpress → releaseが発生した場合，
 * 1回のConsumeでpressedとreleasedが両立することを確認
 *
 * ConsumeStateForTick()はフレームのスナップショットをそのまま渡すため，
 * KEYDOWN後にKEYUPが来た場合は1tickにpressed/releasedが同時に立つ
 */
TEST(InputSystemTest, PressRelease_SameFrame_BothSetInTick){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_Z)); // Shoot押下
    sys.HandleEvent(MakeKeyUp(SDL_SCANCODE_Z));   // 同フレーム内で離す
    const auto tick1 = sys.ConsumeStateForTick();
    EXPECT_TRUE(tick1.IsPressed(Action::Shoot));   // pressedが立っている
    EXPECT_TRUE(tick1.IsReleased(Action::Shoot));  // releasedも立っている
    EXPECT_FALSE(tick1.IsHeld(Action::Shoot));     // すでに離されているのでheld=false
}

// ──────────────────────────────────────────────────────
// 無関係なイベント
// ──────────────────────────────────────────────────────
/**
 * @brief マッピングされていないキーが無視されることを確認
 */
TEST(InputSystemTest, UnmappedKey_Ignored){
    InputSystem sys;
    sys.HandleEvent(MakeKeyDown(SDL_SCANCODE_F1));
    // 何のActionにも影響しない
    EXPECT_FALSE(sys.IsPressed(Action::MoveUp));
    EXPECT_FALSE(sys.IsHeld(Action::MoveUp));
}
