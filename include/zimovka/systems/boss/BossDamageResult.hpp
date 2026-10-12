#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSDAMAGERESULT_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSDAMAGERESULT_HPP_

namespace zimovka{

/**
 * @brief TakeDamageに対する結果用enum
 * 
 */
enum class BossDamageResult{
    InvalidTarget, 
    Damaged, 
    Destroyed, 
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSDAMAGERESULT_HPP_