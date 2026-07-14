#ifndef ZIMOVKA_REPLAY_RUNRECORDER_HPP_
#define ZIMOVKA_REPLAY_RUNRECORDER_HPP_

#include <cstdint>
#include <vector>

namespace zimovka{
/**
 * @brief フレームごとの入力を保持する構造体
 * 
 */
struct RecorderInputFrame{
    std::uint32_t held_bits     = 0;
    std::uint32_t pressed_bits  = 0;
    std::uint32_t released_bits = 0;
};

/**
 * @brief リプレイで用いるゲーム開始〜終了までの記録
 * 
 */
struct RunRecord{
    std::uint32_t format_version = 1;
    std::uint32_t random_seed = 0;
    std::vector<RecorderInputFrame> frames;
};

}   // namespace zimovka

#endif  // ZIMOVKA_REPLAY_RUNRECORDER_HPP_
