#ifndef ZIMOVKA_INPUT_ACTION_HPP_
#define ZIMOVKA_INPUT_ACTION_HPP_

#include <cstddef>
#include <cstdint>

namespace zimovka{

/**
 * @brief プレイヤーが実施できる操作
 * 
 */
enum class Action : std::size_t{
    MoveUp, 
    MoveDown, 
    MoveLeft, 
    MoveRight, 
    Slow, 
    Shoot, 
    Bomb, 
    Pause, 
    Quit, 

    Count,  // ガード用
};

// Actionの要素数取得用のエイリアスACTION_COUNTを用意
static inline constexpr std::size_t ACTION_COUNT = static_cast<std::size_t>(Action::Count);

// Actionの要素をリプレイ用にBit Maskに変換し0/1で持つ
static inline constexpr std::uint32_t ActionBit(Action act){
    /**
     * Actionの値を8ビット(符号なし)整数に変換
     * 左シフトで8ビット整数を表現する位置に1を立てる
     * 例：act=MoveLeft→static_cast<std::uint8_t>(act)=2
     * → 1u << 2 となるので左へ2移動するので，8ビット表記すると0b00000100(10進数で4)
     * ※直感的には0b0000 0010(10進数で2)にしたくなるが，0番目の要素(MoveUp)を表現する桁がないので左シフトしている 
     * */
    return 1u << static_cast<std::uint8_t>(act);
}

}   // namespace zimovka

#endif  // ZIMOVKA_INPUT_ACTION_HPP_
