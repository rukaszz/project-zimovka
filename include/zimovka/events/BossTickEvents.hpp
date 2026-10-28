#ifndef ZIMOVKA_EVENTS_BOSSTICKEVENTS_HPP_
#define ZIMOVKA_EVENTS_BOSSTICKEVENTS_HPP_

namespace zimovka{
/**
 * @brief ボスの演出完了を示す構造体
 * 
 * 撃破演出開始など，1回のみのイベントを管理する
 */
struct BossTickEvents{
    bool defeat_animation_completed = false;
};
} // namespace zimovka

#endif  // ZIMOVKA_EVENTS_BOSSTICKEVENTS_HPP_