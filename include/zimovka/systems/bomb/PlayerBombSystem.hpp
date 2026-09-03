#ifndef ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBSYSTEM_HPP_

#include <cstddef>

#include "zimovka/input/InputState.hpp"
#include "zimovka/systems/bomb/PlayerBombEvents.hpp"
#include "zimovka/systems/bomb/PlayerBombState.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/enemy/EnemySystem.hpp"

namespace zimovka{
/**
 * @brief プレイヤーのボムに関する処理を管理するクラス
 *
 */
class PlayerBombSystem{
private:
    PlayerBombState state_;

    // ボム発動の共通処理
    std::size_t Activate(BulletSystem& enemy_bullets, EnemySystem& enemy_system) noexcept;

public:
    // 状態のリセット
    void Reset() noexcept;
    // 更新(衝突検出後に呼ぶ)
    PlayerBombEvents UpdateTick(
        const InputState& input,
        bool              player_hit,
        BulletSystem&     enemy_bullets,
        EnemySystem&      enemy_system
    );
    // getter
    const PlayerBombState& GetState() const noexcept{
        return state_;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBSYSTEM_HPP_
