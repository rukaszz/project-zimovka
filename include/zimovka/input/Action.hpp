#ifndef ZIMOVKA_INPUT_ACTION_HPP_
#define ZIMOVKA_INPUT_ACTION_HPP_

#include <cstddef>
#include <cstdint>

namespace zimovka{

/**
 * @brief プレイヤーが実施できる操作
 * 
 */
enum class Action : std::uint8_t{
    MoveUp, 
    MoveDown, 
    MoveLeft, 
    MoveRight, 
    Slow, 
    Shoot, 
    Bomb, 
    Pause, 
    Quit, 

    Count,  // ガード用※実操作ではない
};

// Actionの要素数取得用のエイリアスACTION_COUNTを用意
inline constexpr std::uint8_t ACTION_COUNT = static_cast<std::uint8_t>(Action::Count);
// チェック
static_assert(ACTION_COUNT <= 32, "InputState uses 32-bit action masks.");

/**
 * @brief Actionの要素をリプレイ用にBit Maskに変換し0/1で持つための関数
 * 
 * @param act 
 * @return constexpr std::uint32_t 
 */
inline constexpr std::uint32_t ActionBit(Action act){
    /**
     * Actionの値を8ビット(符号なし)整数に変換
     * 左シフトで8ビット整数を表現する位置に1を立てる
     * Action の enum 値を bit index として使う．
     * MoveUp = 0ならbit 0，MoveLeft = 2ならbit 2が立つ。
     */
    return 1u << static_cast<std::uint8_t>(act);
}

}   // namespace zimovka

#endif  // ZIMOVKA_INPUT_ACTION_HPP_
