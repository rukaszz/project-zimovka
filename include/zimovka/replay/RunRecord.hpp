#ifndef ZIMOVKA_REPLAY_RUNRECORD_HPP_
#define ZIMOVKA_REPLAY_RUNRECORD_HPP_

#include <cstdint>
#include <vector>

#include "zimovka/config/SimulationConfig.hpp"

namespace zimovka{

inline constexpr std::uint32_t RUN_RECORD_FORMAT_VERSION = 1;

/**
 * @brief フレームごとの入力を保持する構造体
 * 
 */
struct RecordedInputFrame{
    std::uint32_t held_bits     = 0;
    std::uint32_t pressed_bits  = 0;
    std::uint32_t released_bits = 0;
};

/**
 * @brief リプレイで用いるゲーム開始〜終了までの記録
 * 
 */
struct RunRecord{
    std::uint32_t format_version = RUN_RECORD_FORMAT_VERSION;   // フォーマットのバージョン(リプレイ時の判別用)
    std::uint32_t simulation_hz = zimovka::SimulationConfig::SIMULATION_HZ;   // 記録時のゲームの想定fps
    std::uint32_t random_seed = 0;
    bool frame_limit_reached = false; // 記録の終了地点を示す

    std::vector<RecordedInputFrame> frames;
};

}   // namespace zimovka

#endif  // ZIMOVKA_REPLAY_RUNRECORD_HPP_
