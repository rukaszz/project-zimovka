#ifndef ZIMOVKA_SYSTEMS_BOSS_BOSSSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_BOSS_BOSSSYSTEM_HPP_

#include <cstdint>

#include "zimovka/events/BossTickEvents.hpp"
#include "zimovka/systems/boss/Boss.hpp"
#include "zimovka/systems/boss/BossConfig.hpp"
#include "zimovka/systems/boss/BossEvents.hpp"
#include "zimovka/systems/boss/BossDamageResult.hpp"
#include "zimovka/systems/boss/BossAttackStep.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/pattern/PatternSystem.hpp"

namespace zimovka{
/**
 * @brief ボス関係の処理(API)管理用クラス
 * 
 * Enemyとは性質が似て非なるものであるため継承はしない
 */
class BossSystem{
private: 
    Boss boss_;

public:
    // ボス戦開始処理
    void Start(const BossConfig& config);
    // リセット処理
    void Reset() noexcept;
    // 更新
    BossTickEvents UpdateTick(
        float dt,
        const Vec2& player_position,
        PatternSystem& patterns,
        BulletSystem& enemy_bullets
    );
    // ダメージを受ける処理
    BossDamageResult TakeDamage(std::int32_t damage);
    
    // getter
    [[nodiscard]]
    const Boss& GetBoss() const noexcept{
        return boss_;
    }
    [[nodiscard]]
    bool IsNotInactive() const noexcept{
        return boss_.status != BossStatus::Inactive;
    }
    [[nodiscard]]
    bool IsFighting() const noexcept{
        return boss_.status == BossStatus::Fighting;
    }
    [[nodiscard]]
    bool IsDefeating() const noexcept{
        return boss_.status == BossStatus::Defeating;
    }
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOSS_BOSSSYSTEM_HPP_
