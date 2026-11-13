#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSDAMAGERESULT_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSDAMAGERESULT_HPP_

namespace zimovka{

/**
 * @brief TakeDamageに対する結果用enum
 * 
 */
enum class BossDamageResult{
    InvalidTarget,      // ダメージ不成立
    Damaged,            // 現在のフェーズのHPが減少
    PhaseComplete,      // 現在フェーズの終了と次のフェーズ
    Destroyed,          // 最終フェーズが終了(ボスを撃破)
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSDAMAGERESULT_HPP_
