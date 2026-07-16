#ifndef ZIMOVKA_CPRE_SIMULATIONCONFIG_HPP_
#define ZIMOVKA_CPRE_SIMULATIONCONFIG_HPP_

#include <chrono>
#include <cstdint>

/**
 * @brief リプレイ時のゲーム環境を再現するための設定
 * 
 */
namespace zimovka{
namespace SimulationConfig{

// 再現するfps
inline constexpr std::uint32_t SIMULATION_HZ = 60;
// SIMULATION_HZに対する，1フレーム分の時間
inline constexpr float FIXED_DELTA_SECONDS = 1.0f/static_cast<float>(SIMULATION_HZ); 
// ナノ秒単位のステップ時間
inline constexpr std::chrono::nanoseconds FIXED_STEP{
    1'000'000'000LL / SIMULATION_HZ
};
}   // namespace SimulationConfig
}   // namespace zimovka

#endif  // ZIMOVKA_CPRE_SIMULATIONCONFIG_HPP_
