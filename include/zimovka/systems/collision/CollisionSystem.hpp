#ifndef ZIMOVKA_SYSTEMS_COLLISION_COLLISIONSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_COLLISION_COLLISIONSYSTEM_HPP_

#include <cstddef>

#include "zimovka/core/Circle.hpp"
#include "zimovka/events/EnemyHitEvents.hpp"
#include "zimovka/systems/collision/CollisionStats.hpp"
#include "zimovka/systems/player/Player.hpp"
#include "zimovka/systems/enemy/EnemySystem.hpp"
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
    // 衝突判定集計用
    CollisionStats collision_stats_{};

public:
    // プレイヤー vs 弾のヒットチェック
    bool CheckPlayerHitByBullets(const Player& player, const BulletSystem& bullets);
    // 自機弾 vs 敵のヒットチェック・解決
    EnemyHitEvents ResolvePlayerBulletVsEnemies(BulletSystem& player_bullets, EnemySystem& enemies);
    // getter
    std::size_t LastCheckCount() const noexcept{
        return last_check_count_;
    }
    void InitializeStatsAtBeginTick() noexcept{
        collision_stats_ = {};
    }
    const CollisionStats& GetStats() const noexcept{
        return collision_stats_;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_COLLISION_COLLISIONSYSTEM_HPP_
