#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSEVENTS_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSEVENTS_HPP_

namespace zimovka{
/**
 * @brief ボスのイベント管理用
 * 
 * DefeatAnimationCompleteのような1Tickのみのイベントはここで管理しない
 */
enum class BossEvents{
    None,
    WaitingTrigger,
    Active,
    Defeated,                // 撃破演出進行中
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSEVENTS_HPP_
