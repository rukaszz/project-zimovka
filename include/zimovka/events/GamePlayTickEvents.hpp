#ifndef ZIMOVKA_EVENTS_GAMEPLAYTICKEVENTS_HPP_
#define ZIMOVKA_EVENTS_GAMEPLAYTICKEVENTS_HPP_

#include "zimovka/events/PlayerWeaponEvents.hpp"

namespace zimovka{
/**
 * @brief GamePlay中のイベントを管理する
 * 
 */
struct GamePlayTickEvents{
    bool player_hit = false;
    PlayerWeaponEvents weapon{};
};

}   // namespace zimovka

#endif  // ZIMOVKA_EVENTS_GAMEPLAYTICKEVENTS_HPP_
