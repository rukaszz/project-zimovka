#ifndef ZIMOVKA_INPUT_INPUTSTATE_HPP_
#define ZIMOVKA_INPUT_INPUTSTATE_HPP_

#include <cstdint>

#include "zimovka/input/Action.hpp"

namespace zimovka{

/**
 * @brief 1フレーム分の入力状態スナップショット
 * Bit Maskを用いて0, 1の並びで管理する
 * ※リプレイ時は1フレーム分の入力値を整数値として記録していく
 * 
 * held     : 押され続けている
 * pressed  : このフレームで押された
 * released : このフレームで入力解除された
 * 
 */
class InputState{
private:
    std::uint32_t held_bits_ = 0;
    std::uint32_t pressed_bits_ = 0;
    std::uint32_t released_bits_ = 0;

public:
    // 状態の確認
    // 0/1 and 0/1 のビット演算を実施して0ではない = 1(押されている)としてtrueになる
    bool IsHeld(zimovka::Action act) const noexcept;
    bool IsPressed(zimovka::Action act) const noexcept;
    bool IsReleased(zimovka::Action act) const noexcept;
    // 状態の参照
    // 現在の各Actionの押されている状態を取得
    std::uint32_t GetHeldBits() const noexcept;
    std::uint32_t GetPressedBits() const noexcept;
    std::uint32_t GetReleasedBits() const noexcept;
    // 押した/離れたの一時的な状態を初期化
    void ClearTransient() noexcept;
    /**
     * 注意
     * このsetter群は外部から呼べてしまう
     * InputSystemが入力状態を構築するための更新API．
     * ゲームロジック側には const InputState& のみを渡す．
     */
    // 押され続けているか判定
    void SetHeld(zimovka::Action act, bool val) noexcept;
    // 押されたか判定
    void SetPressed(zimovka::Action act) noexcept;
    // 離されたか判定
    void SetReleased(zimovka::Action act) noexcept;
    /**
     * @brief Bit MaskからInputState型オブジェクトを作成する
     * コンストラクタを介さず外部からBit Maskを渡してInputStateを作れる
     * いわゆる代替コンストラクタ
     * 
     * @param held_bits 
     * @param pressed_bits 
     * @param released_bits 
     * @return InputState 
     */
    static InputState FromBits(
        std::uint32_t held_bits, 
        std::uint32_t pressed_bits, 
        std::uint32_t released_bits 
    ) noexcept 
    {
        InputState state;
        state.held_bits_ = held_bits;
        state.pressed_bits_ = pressed_bits;
        state.released_bits_ = released_bits;
        return state;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_INPUT_INPUTSTATE_HPP_
