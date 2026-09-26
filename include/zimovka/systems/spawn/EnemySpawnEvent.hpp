#ifndef ZIMOVKA_SYSTEMS_SPAWN_ENEMYSPAWNEVENT_HPP_
#define ZIMOVKA_SYSTEMS_SPAWN_ENEMYSPAWNEVENT_HPP_

#include <cstdint>

#include "zimovka/systems/enemy/EnemySpawnParams.hpp"

namespace zimovka{
/**
 * @brief 敵スポーンイベント呼び出し用構造体
 * 
 */
struct EnemySpawnEvent{
    std::uint64_t tick         = 0; // 呼び出すTick
    EnemySpawnParams params{};
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_SPAWN_ENEMYSPAWNEVENT_HPP_
