#include "zimovka/input/InputSystem.hpp"

namespace zimovka{

/**
 * @brief キー押下イベント処理(ProcessEvents()で呼ばれる)
 *
 * @param event
 */
void InputSystem::HandleEvent(const SDL_Event& event){
    // キー押す/離すの操作に応じて関数を呼ぶ
    if(event.type == SDL_KEYDOWN){
        HandleKeyDown(event.key.keysym.scancode, event.key.repeat != 0);
    } else if(event.type == SDL_KEYUP){
        HandleKeyUp(event.key.keysym.scancode);
    } else if(event.type == SDL_WINDOWEVENT &&
              event.window.event == SDL_WINDOWEVENT_FOCUS_LOST){
        // フォーカス喪失時はSDL_KEYUPが届かないため手動で全状態をリセット
        // 参照カウントの整合性を保つためhold_counts_もリセット
        state_.ClearAll();
        hold_counts_.fill(0);
    }
}

/**
 * @brief キーの状態を確認するアクセサ
 *
 * @param act
 * @return true
 * @return false
 */
bool InputSystem::IsHeld(zimovka::Action act) const noexcept{
    return state_.IsHeld(act);
}
bool InputSystem::IsPressed(zimovka::Action act) const noexcept{
    return state_.IsPressed(act);
}
bool InputSystem::IsReleased(zimovka::Action act) const noexcept{
    return state_.IsReleased(act);
}

/**
 * @brief 更新処理時の入力スナップショットを取得する処理
 *
 * 処理遅延で複数回のUpdate()が走る際に入力が失われないようにする
 * Update前に呼ばれることで，1更新と1入力が対応づく
 * 連続して更新処理ループで呼ばれたとしても初回はpressed/released，複数回呼ばれるとheldになる
 *
 * @return InputState
 */
InputState InputSystem::ConsumeStateForTick() noexcept{
    // 現在のInputStateをコピー
    InputState snapshot = state_;
    // 入力削除
    state_.ClearTransient();
    return snapshot;
}

/**
 * @brief キー押下時のInputState制御関数
 *
 * 同一Actionに複数キーが割り当てられている場合は参照カウントで管理する
 * 例: LSHIFT + RSHIFT → Action::Slow
 *   LShiftを押す  : hold_count[Slow] == 0 → SetHeld(true), SetPressed
 *   RShiftも押す  : hold_count[Slow] == 1 → カウントのみ増加
 *   LShiftを離す  : hold_count[Slow] == 1 → カウント減，まだ1なので held は維持
 *   RShiftを離す  : hold_count[Slow] == 0 → SetHeld(false), SetReleased
 *
 * @param scancode
 * @param repeat  event.key.repeat != 0 なので押され続けている→trueになる
 */
void InputSystem::HandleKeyDown(SDL_Scancode scancode, bool repeat){
    // キーリピートなら無視する※連続発火防止
    if(repeat){
        return;
    }
    // Actionにマッピングする
    zimovka::Action act;
    if(!MapKeyToAction(scancode, act)){
        // マッピングできないキーなら即return
        return;
    }
    // 押されたキーに対応するインデックス取得
    const auto idx = static_cast<std::size_t>(act);
    // 同じActionの最初の押下時(押下カウントが0)のみpressed + heldをセット
    if(hold_counts_[idx] == 0){
        // 前フレームで押されていなければ(heldでなければ)Pressed
        if(!state_.IsHeld(act)){
            state_.SetPressed(act);
        }
        state_.SetHeld(act, true);
    }
    // 押されたカウント+1
    ++hold_counts_[idx];
}

/**
 * @brief キーが離れたときのInputState制御関数
 *
 * hold_count_が0になって初めてreleased + heldクリアをセットする
 *
 * @param scancode
 */
void InputSystem::HandleKeyUp(SDL_Scancode scancode){
    // Actionにマッピング
    zimovka::Action act;
    if(!MapKeyToAction(scancode, act)){
        // マッピングできないキーなら即return
        return;
    }
    // 押されたキーに対応するインデックス取得
    const auto idx = static_cast<std::size_t>(act);
    // 押されたカウントが1以上なら-1
    if(hold_counts_[idx] > 0){
        --hold_counts_[idx];
    }
    // 同じActionに対応する全キーが離れた時のみheld解除 + released
    // ex) wキー+↑キーでwキーを離しても上方向へ移動させるため
    if(hold_counts_[idx] == 0){
        state_.SetHeld(act, false);
        state_.SetReleased(act);
    }
}

/**
 * @brief 押下されたキーを確認し引数に渡されたactへマッピングする関数
 *
 * @param scancode
 * @param out_act
 * @return true
 * @return false マッピングできないケースはfalseになる
 */
bool InputSystem::MapKeyToAction(SDL_Scancode scancode, zimovka::Action& out_act) const{
    // scancode確認
    switch (scancode){
    // 矢印キーとWASDどちらがきてもActionに対応付ける
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_W:
        out_act = zimovka::Action::MoveUp;
        return true;
    case SDL_SCANCODE_DOWN:
    case SDL_SCANCODE_S:
        out_act = zimovka::Action::MoveDown;
        return true;
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_A:
        out_act = zimovka::Action::MoveLeft;
        return true;
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_D:
        out_act = zimovka::Action::MoveRight;
        return true;
    // 左右どちらのSHIFTキーにも対応
    case SDL_SCANCODE_LSHIFT:
    case SDL_SCANCODE_RSHIFT:
        out_act = zimovka::Action::Slow;
        return true;
    case SDL_SCANCODE_Z:
        out_act = zimovka::Action::Shoot;
        return true;
    case SDL_SCANCODE_X:
        out_act = zimovka::Action::Bomb;
        return true;
    case SDL_SCANCODE_P:
        out_act = zimovka::Action::Pause;
        return true;
    case SDL_SCANCODE_ESCAPE:
        out_act = zimovka::Action::Quit;
        return true;
    default:
        return false;
    }
}

}   // namespace zimovka
