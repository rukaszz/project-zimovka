#ifndef ZIMOVKA_EVENTS_BOSSHITEVENTS_HPP_
#define ZIMOVKA_EVENTS_BOSSHITEVENTS_HPP_

#include <cstddef>

namespace zimovka{
/**
 * @brief 自機弾がボスに命中した際のイベント
 *
 * SE/スコア加算などを呼び出すためのイベント管理
 * CollisionSystem::ResolvePlayerBulletsVsBoss()から返される
 */
struct BossHitEvents{
    std::size_t hit_count  = 0;     // 命中した弾の数
    bool defeated_boss     = false; // 撃破した敵の数
};

}   // namespace zimovka

#endif  // ZIMOVKA_EVENTS_BOSSHITEVENTS_HPP_
