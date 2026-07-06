#include "zimovka/input/InputState.hpp"

namespace zimovka{

// 状態の確認
/**
 * @brief 各InputState要素の状態を取得するアクセサ
 * 
 * 0/1 and 0/1 のビット演算を実施して0ではない = 1(押されている)としてtrueになる
 * 
 * @param act 
 * @return true 
 * @return false 
 */
bool InputState::IsHeld(zimovka::Action act) const noexcept{
    return (held_bits_ & ActionBit(act)) != 0;
}
bool InputState::IsPressed(zimovka::Action act) const noexcept{
    return (pressed_bits_ & ActionBit(act)) != 0;
}
bool InputState::IsReleased(zimovka::Action act) const noexcept{
    return (released_bits_ & ActionBit(act)) != 0;
}
// 状態の参照
/**
 * @brief 現在のInputStateのデータを取得するアクセサ
 * 
 * 現在の各Actionの押されている状態を取得する
 * 
 * @return std::uint32_t 
 */
std::uint32_t InputState::GetHeldBits() const noexcept{
    return held_bits_;
}
std::uint32_t InputState::GetPressedBits() const noexcept{
    return pressed_bits_;
}
std::uint32_t InputState::GetReleasedBits() const noexcept{
    return released_bits_;
}

/**
 * @brief InputStateの押した/離れたの一時的な状態を初期化する関数
 * 
 * フレーム開始時に呼ばれる
 * 
 */
void InputState::ClearTransient() noexcept{
    pressed_bits_ = 0;
    released_bits_ = 0;
}

/**
 * @brief 全入力状態を初期化する関数
 *
 * フォーカス喪失時などにSDL_KEYUPが届かないケースで呼ばれる
 * held_bitsも含めて全クリアする
 */
void InputState::ClearAll() noexcept{
    held_bits_     = 0;
    pressed_bits_  = 0;
    released_bits_ = 0;
}

/**
 * 注意
 * このsetter群は外部から呼べてしまう
 * friendクラスにする代わりに運用で外部から呼ばないようにすること
 */
/**
 * @brief 特定のActionが押され続けている状態(held)に設定する
 * 
 * @param act 
 * @param val 
 */
void InputState::SetHeld(Action act, bool val) noexcept{
    // Actionのビット変換
    const std::uint32_t bit = ActionBit(act);
    // valで押されている/押されていないを切り替える
    if(val){
        // OR演算でどちらかが立っているなら1になる
        held_bits_ |= bit;
    } else {
        // ~bitでそのビットが0になるので，AND演算でそのactのビットのみ0になる
        held_bits_ &= ~bit;
    }
}
// 押された
void InputState::SetPressed(Action act) noexcept{
    pressed_bits_ |= ActionBit(act);
}
// 離された
void InputState::SetReleased(Action act) noexcept{
    released_bits_ |= zimovka::ActionBit(act);
}

}   // namespace zimovka
