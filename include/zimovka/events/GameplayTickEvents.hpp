#ifndef ZIMOVKA_EVENTS_GAMEPLAYTICKEVENTS_HPP_
#define ZIMOVKA_EVENTS_GAMEPLAYTICKEVENTS_HPP_

#include "zimovka/events/PlayerWeaponEvents.hpp"
#include "zimovka/events/EnemyHitEvents.hpp"

namespace zimovka{
/**
 * @brief GamePlay中のイベントを管理する
 *
 */
struct GameplayTickEvents{
    bool               player_hit = false;
    EnemyHitEvents     enemy_hit{};
    PlayerWeaponEvents weapon{};
};

}   // namespace zimovka

#endif  // ZIMOVKA_EVENTS_GAMEPLAYTICKEVENTS_HPP_
