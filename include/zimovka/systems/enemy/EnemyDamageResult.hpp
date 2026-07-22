#ifndef ZIMOVKA_SYSTEMS_ENEMY_ENEMYDAMAGERESULT_HPP_
#define ZIMOVKA_SYSTEMS_ENEMY_ENEMYDAMAGERESULT_HPP_

namespace zimovka{

/**
 * @brief TakeDamageに対する結果用enum
 * 
 */
enum class EnemyDamageResult{
    InvalidTarget, 
    Damaged, 
    Destroyed, 
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_ENEMY_ENEMYDAMAGERESULT_HPP_