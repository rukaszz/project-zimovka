#ifndef ZIMOVKA_STAGES_STAGEPHASE_HPP_
#define ZIMOVKA_STAGES_STAGEPHASE_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief ステージの進行状態を管理するenum
 * 
 */
enum class StageStatement : std::uint8_t{
    Normal, 
    Boss, 
    Cleared, 
};
/**
 * @brief ステージの状態管理用構造体
 * 
 */
struct StagePhase{
    std::uint64_t stage_clear_ticks_elapsed = 0;
    StageStatement stage_state = StageStatement::Normal;
};
} // namespace zimovka

#endif  // ZIMOVKA_STAGES_STAGEPHASE_HPP_
