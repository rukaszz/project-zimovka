#ifndef ZIMOVKA_SYSTEMS_PLAYER_PLAYERWEAPONSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_PLAYER_PLAYERWEAPONSYSTEM_HPP_

#include "zimovka/input/InputState.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/player/Player.hpp"
#include "zimovka/systems/player/PlayerWeaponConfig.hpp"
#include "zimovka/systems/player/PlayerWeaponState.hpp"
#include "zimovka/events/PlayerWeaponEvents.hpp"

namespace zimovka{

class PlayerWeaponSystem{
private:
    // 弾設定
    PlayerWeaponConfig config_;
    // 射撃状態
    PlayerWeaponState state_;
    // リロードを開始する
    void StartReload(PlayerWeaponEvents& events) noexcept;

public:
    // PlayerWeaponConfigのみ受け取るようにexplicit
    explicit PlayerWeaponSystem(PlayerWeaponConfig config = {});
    // 状態のリセット
    void Reset() noexcept;
    // 更新
    PlayerWeaponEvents UpdateTick(
        const InputState& input, 
        const Player& player, 
        BulletSystem& player_bullets
    );
    // getter
    const PlayerWeaponConfig& GetConfig() const noexcept{
        return config_;
    }
    const PlayerWeaponState& GetState() const noexcept{
        return state_;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_PLAYER_PLAYERWEAPONSYSTEM_HPP_
