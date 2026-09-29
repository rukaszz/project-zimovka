#ifndef ZIMOVKA_EVENTS_BULLETCANCELEVENTS_HPP_
#define ZIMOVKA_EVENTS_BULLETCANCELEVENTS_HPP_

#include <cstddef>

namespace zimovka{
/**
 * @brief 自機弾vs敵弾による打ち消しイベント管理用
 * 
 */
struct BulletCancelEvents{
    std::size_t cancel_count = 0;
};
} // namespace zimovka

#endif  // ZIMOVKA_EVENTS_BULLETCANCELEVENTS_HPP_
