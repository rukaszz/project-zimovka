#ifndef ZIMOVKA_INPUT_INPUTSYSTEM_HPP_
#define ZIMOVKA_INPUT_INPUTSYSTEM_HPP_

#include <array>
#include <cstdint>

#include <SDL2/SDL.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"

namespace zimovka{

/**
 * @brief SDL入力をゲーム内Actionへ変換し，次の固定更新で消費される入力状態を管理するクラス
 *
 */
class InputSystem{
private:
    // Bit Mask入力管理オブジェクトstate
    zimovka::InputState state_;
    /**
     * @brief 同一Actionに複数キーが割り当てられている場合の参照カウントを保持する配列
     *
     * 例: LSHIFT/RSHIFTは両方Action::Slowにマッピングされる
     * hold_count_[Slow] == 2 → どちらかが離れても解除しない
     * hold_count_[Slow] == 0 になって初めてheld/releasedを更新する
     */
    std::array<std::uint8_t, ACTION_COUNT> hold_counts_{};

    /**
     * @brief 物理的なキーの押下状態を管理する配列
     * 
     * 対応する物理キーの状態を保持し，キーリピートではない同一物理キーの入力を無視する
     * 物理キーの状態も確認することで同一キーの入力カウントの重複を防止する
     */
    std::array<bool, SDL_NUM_SCANCODES> physical_key_held_{};

public:
    // イベント連携用
    void HandleEvent(const SDL_Event& event);
    // 状態の参照
    bool IsHeld(zimovka::Action act)     const noexcept;
    bool IsPressed(zimovka::Action act)  const noexcept;
    bool IsReleased(zimovka::Action act) const noexcept;

    // 複数回のUpdate実行時に入力をスナップショットとして記録する関数
    InputState ConsumeStateForTick() noexcept;

private:
    // キーの入力状態管理関数
    void HandleKeyDown(SDL_Scancode scancode, bool repeat);
    void HandleKeyUp(SDL_Scancode scancode);
    // キー入力とActionのマッピング関数
    bool MapKeyToAction(SDL_Scancode scancode, zimovka::Action& out_act) const;
};

}   // namespace zimovka


#endif  // ZIMOVKA_INPUT_INPUTSYSTEM_HPP_
