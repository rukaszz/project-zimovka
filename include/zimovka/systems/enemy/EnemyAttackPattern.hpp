#ifndef ZIMOVKA_SYSTEM_ENEMY_ENEMYATTACKPATTERN_HPP_
#define ZIMOVKA_SYSTEM_ENEMY_ENEMYATTACKPATTERN_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief 敵の攻撃パターンを定義
 * 
 * ※敵の種類ではなく攻撃パターン
 */
enum class EnemyAttackPattern : std::uint8_t{
    AimedSpread,    // 自機狙い拡散
    FixedSpread,    // 自機を狙わない拡散

    Count,          // 門番
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEM_ENEMY_ENEMYATTACKPATTERN_HPP_
