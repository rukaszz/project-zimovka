#ifndef ZIMOVKA_EVENTS_ENEMYHITEVENTS_HPP_
#define ZIMOVKA_EVENTS_ENEMYHITEVENTS_HPP_

#include <cstddef>

namespace zimovka{
/**
 * @brief 自機弾が敵に命中した際のイベント
 *
 * SE/スコア加算などを呼び出すためのイベント管理
 * CollisionSystem::ResolvePlayerBulletsVsEnemies()から返される
 */
struct EnemyHitEvents{
    std::size_t hit_count  = 0;  // 命中した弾の数
    std::size_t kill_count = 0;  // 撃破した敵の数
};

}   // namespace zimovka

#endif  // ZIMOVKA_EVENTS_ENEMYHITEVENTS_HPP_
