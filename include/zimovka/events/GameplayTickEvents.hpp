#ifndef ZIMOVKA_EVENTS_GAMEPLAYTICKEVENTS_HPP_
#define ZIMOVKA_EVENTS_GAMEPLAYTICKEVENTS_HPP_

#include "zimovka/events/PlayerWeaponEvents.hpp"
#include "zimovka/events/EnemyHitEvents.hpp"
#include "zimovka/systems/bomb/PlayerBombEvents.hpp"

namespace zimovka{
/**
 * @brief GamePlay中のイベントを管理する
 *
 */
struct GameplayTickEvents{
    bool               player_hit = false;  // 被弾確定フラグ(ボム解決後)
    EnemyHitEvents     enemy_hit{};
    PlayerWeaponEvents weapon{};
    PlayerBombEvents   bomb{};
};

}   // namespace zimovka

#endif  // ZIMOVKA_EVENTS_GAMEPLAYTICKEVENTS_HPP_
