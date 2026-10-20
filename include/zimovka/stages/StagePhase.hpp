#ifndef ZIMOVKA_STAGES_STAGEPHASE_HPP_
#define ZIMOVKA_STAGES_STAGEPHASE_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief ステージの進行状態を管理するenum
 * 
 */
enum class StagePhase : std::uint8_t{
    Normal, 
    Boss, 
    Cleared, 
};
} // namespace zimovka

#endif  // ZIMOVKA_STAGES_STAGEPHASE_HPP_
