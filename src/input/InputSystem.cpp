#include "zimovka/input/InputSystem.hpp"

/**
 * @brief 入力状態(InputState)を初期化する
 * 
 * ゲームループ開始時に呼ばれる
 * 
 */
void zimovka::InputSystem::BeginFrame(){
    state_.ClearTransient();
}

void zimovka::InputSystem::HandleEvent(const SDL_Event& event){
    // キーが押された/離れたでそれぞれ関数を呼び出す
    if(event.type == SDL_KEYDOWN){
        HandleKeyDown(event.key.keysym.scancode, event.key.repeat != 0);
    } else if(event.type == SDL_KEYUP){
        HandleKeyUp(event.key.keysym.scancode);
    }
}

/**
 * @brief キーの状態を確認するアクセサ
 * 
 * @param act 
 * @return true 
 * @return false 
 */
bool zimovka::InputSystem::IsHeld(zimovka::Action act) const noexcept{
    return state_.IsHeld(act);
}
bool zimovka::InputSystem::IsPressed(zimovka::Action act) const noexcept{
    return state_.IsPressed(act);
}
bool zimovka::InputSystem::IsReleased(zimovka::Action act) const noexcept{
    return state_.IsReleased(act);
}

/**
 * @brief キー押下時のInputState制御関数
 * 
 * @param scancode 
 * @param repeat  event.key.repeat != 0 なので押され続けている→trueになる
 */
void zimovka::InputSystem::HandleKeyDown(SDL_Scancode scancode, bool repeat){
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
    // 前のフレームから押されているキーでなければpressed
    if(!state_.IsHeld(act)){
        state_.SetPressed(act);
    }
    // 押されている判定ON
    state_.SetHeld(act, true);
}

/**
 * @brief キーが離れたときのInputState制御関数
 * 
 * @param scancode 
 */
void zimovka::InputSystem::HandleKeyUp(SDL_Scancode scancode){
    // Actionにマッピング
    zimovka::Action act;
    if(!MapKeyToAction(scancode, act)){
        // マッピングできないキーなら即return
        return;
    }
    // 押されている判定OFF
    state_.SetHeld(act, false);
    state_.SetReleased(act);
}

/**
 * @brief 押下されたキーを確認し引数に渡されたactへマッピングする関数
 * 
 * @param scancode 
 * @param out_act 
 * @return true 
 * @return false マッピングできないケースはfalseになる 
 */
bool zimovka::InputSystem::MapKeyToAction(SDL_Scancode scancode, zimovka::Action& out_act) const{
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
