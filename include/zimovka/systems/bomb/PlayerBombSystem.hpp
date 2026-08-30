#ifndef ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBSYSTEM_HPP_

#include "zimovka/input/InputState.hpp"
#include "zimovka/systems/bomb/PlayerBombEvents.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"

namespace zimovka{
/**
 * @brief プレイヤーのボムに関する処理を管理するクラス
 * 
 */
class PlayerBombSystem{
public:
    PlayerBombEvents UpdateTick(
        const InputState& input,
        BulletSystem& enemy_bullets
    );
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBSYSTEM_HPP_
