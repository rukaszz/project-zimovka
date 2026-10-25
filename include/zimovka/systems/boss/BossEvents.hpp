#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSEVENTS_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSEVENTS_HPP_

namespace zimovka{
/**
 * @brief ボスのイベント管理用
 * 
 */
enum class BossEvents{
    None,
    WaitingTrigger,
    Active,
    Defeated,                // 撃破演出進行中
    DefeatAnimationComplete, // 撃破演出完了(ステージクリアのトリガ)
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSEVENTS_HPP_
