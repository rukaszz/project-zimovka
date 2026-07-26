#ifndef ZIMOVKA_SYSTEMS_COLLISION_COLLISIONSTATS_HPP_
#define ZIMOVKA_SYSTEMS_COLLISION_COLLISIONSTATS_HPP_

#include <cstddef>

namespace zimovka{
/**
 * @brief PlayerやEnemyの衝突判定の値を集計する
 * 
 */
struct CollisionStats{
    std::size_t player_vs_enemy_bullet_checks = 0;
    std::size_t player_bullet_vs_enemy_checks = 0;
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_COLLISION_COLLISIONSTATS_HPP_
