#ifndef ZIMOVKA_INPUT_INPUTSTATE_HPP_
#define ZIMOVKA_INPUT_INPUTSTATE_HPP_

#include <cstdint>

#include "zimovka/input/Action.hpp"

namespace zimovka{

/**
 * @brief 1フレーム文の入力状態スナップショット
 * →配列で{0, 1, 1, 0, 0}みたく管理する
 * ※リプレイ機能のためにビット管理する
 * 
 * held     : 押され続けている
 * pressed  : このフレームで押された
 * released : このフレームで離れた
 * 
 */
class InputState{
private:
    std::uint32_t heldBits_ = 0;
    std::uint32_t pressedBits_ = 0;
    std::uint32_t releasedBits_ = 0;

public:
    // 状態の確認
    // 0/1 and 0/1 のビット演算を実施して0ではない = 1(押されている)としてtrueになる
    bool IsHeld(zimovka::Action act) const noexcept{
        return (heldBits_ & ActionBit(act)) != 0;
    }
    bool IsPressed(zimovka::Action act) const noexcept{
        return (pressedBits_ & ActionBit(act)) != 0;
    }
    bool IsReleased(zimovka::Action act) const noexcept{
        return (releasedBits_ & ActionBit(act)) != 0;
    }
    // 状態の参照
    // 現在の各Actionの押されている状態を取得
    std::uint32_t GetHeldBits() const noexcept{
        return heldBits_;
    }
    std::uint32_t GetPressedBits() const noexcept{
        return pressedBits_;
    }
    std::uint32_t GetReleasedBits() const noexcept{
        return releasedBits_;
    }
    // 押した/離れたの一時的な状態を初期化
    void ClearTransient() noexcept{
        pressedBits_ = 0;
        releasedBits_ = 0;
    }
    /**
     * 注意
     * このsetter群は外部から呼べてしまう
     * friendクラスにする代わりに運用で外部から呼ばないように防止する
     */
    // 押され続けているか判定
    void SetHeld(Action act, bool val) noexcept{
        // Actionのビット変換
        const std::uint32_t bit = ActionBit(act);
        // valで押されている/押されていないを切り替える
        if(val){
            // OR演算でどちらかが立っているなら1になる
            heldBits_ |= bit;
        } else {
            // ~bitでそのビットが0になるので，AND演算でactのビットのみ0になる
            heldBits_ &= ~bit;
        }
    }
    // 押されたか判定
    void SetPressed(Action act) noexcept{
        pressedBits_ |= ActionBit(act);
    }
    // 離されたか判定
    void SetReleased(Action act) noexcept{
        releasedBits_ |= zimovka::ActionBit(act);
    }
    static InputState FromBits(
        std::uint32_t heldBits, 
        std::uint32_t pressedBits, 
        std::uint32_t releaseBits 
    ) noexcept 
    {
        InputState state;
        state.heldBits_ = heldBits;
        state.pressedBits_ = pressedBits;
        state.releasedBits_ = releaseBits;
        return state;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_INPUT_INPUTSTATE_HPP_
