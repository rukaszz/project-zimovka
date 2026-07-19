#ifndef ZIMOVKA_SYSTEMS_PLAYER_PLAYERWEAPONSTATE_HPP_
#define ZIMOVKA_SYSTEMS_PLAYER_PLAYERWEAPONSTATE_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief プレイヤーの弾発射に関する状態管理用構造体
 * 
 */
struct PlayerWeaponState{
    // 残弾
    std::uint32_t ammo = 0;
    // 発射/リロードなどのクールダウン処理の残tick
    std::uint32_t cooldown_ticks_remaining = 0;
    std::uint32_t reload_ticks_remaining   = 0;

    /**
     * @brief リロード中か
     * 
     * @return true 
     * @return false 
     */
    bool IsReloading() const noexcept{
        return reload_ticks_remaining > 0;
    }
    /**
     * @brief プレイヤーが射撃可能か
     * 
     * @return true 
     * @return false 
     */
    bool IsReadyToFire() const noexcept{
        return ammo > 0
            && cooldown_ticks_remaining == 0
            && reload_ticks_remaining   == 0;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_PLAYER_PLAYERWEAPONSTATE_HPP_
