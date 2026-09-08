#ifndef ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBCONFIG_HPP_
#define ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBCONFIG_HPP_

#include <cstdint>

/**
 * @brief プレイヤーのボムの設定
 * 
 */
namespace zimovka{
namespace PlayerBombConfig{
    // ボム発動中の無敵時間3秒(60Hz simulationで180tick = 3秒)
    inline constexpr std::uint32_t INVINCIBLE_TICKS = 180;
    // 食らいボム受け付けウィンドウ※約133ms(simulation tick rate 60)
    inline constexpr std::uint32_t GRACE_TICKS = 8;
    // ボムで敵に与えるダメージ(実質即死)
    inline constexpr std::int32_t  BOMB_DAMAGE = 9999;
} // namespace PlayerBombConfig
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBCONFIG_HPP_
