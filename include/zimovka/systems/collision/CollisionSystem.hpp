#ifndef ZIMOVKA_SYSTEMS_COLLISION_COLLISIONSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_COLLISION_COLLISIONSYSTEM_HPP_

#include <cstddef>

#include "zimovka/core/Circle.hpp"
#include "zimovka/systems/enemy/Enemy.hpp"
#include "zimovka/systems/player/Player.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"

namespace zimovka{

/**
 * @brief オブジェクト同士の衝突に関するシステム
 * 
 */
class CollisionSystem{
private:
    // activeな弾のチェック回数
    std::size_t last_check_count_ = 0;
public:
    // プレイヤー vs 弾のヒットチェック
    bool CheckPlayerHitByBullets(const Player& player, const BulletSystem& bullets);
    // getter
    std::size_t LastCheckCount() const noexcept{
        return last_check_count_;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_COLLISION_COLLISIONSYSTEM_HPP_
