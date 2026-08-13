#ifndef ZIMOVKA_ENGINE_UPDATE_UPDATEPIPELINE_HPP_
#define ZIMOVKA_ENGINE_UPDATE_UPDATEPIPELINE_HPP_

#include "zimovka/core/DeterministicRng.hpp"
#include "zimovka/events/GameplayTickEvents.hpp"
#include "zimovka/events/PlayerWeaponEvents.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/collision/CollisionSystem.hpp"
#include "zimovka/systems/enemy/EnemySystem.hpp"
#include "zimovka/systems/player/PlayerSystem.hpp"
#include "zimovka/systems/player/PlayerWeaponSystem.hpp"

namespace zimovka{

// 前方宣言
class PrimitiveRenderer;

/**
 * @brief 1固定Tickあたりの更新順序を管理し各システムのUpdate()を呼び出す
 *
 * 更新順: 自機移動 → 武器/発射 → 弾移動 → 衝突解決
 */
class UpdatePipeline{
private:
    // ワールドのサイズ
    float world_width_  = 0.0f;
    float world_height_ = 0.0f;
    // 決定論的乱数
    DeterministicRng gameplay_rng_{0};
    // 更新tick
    std::uint64_t tick_index_ = 0;
    // プレイヤー関連
    PlayerSystem       player_system_;
    PlayerWeaponSystem player_weapon_system_;
    // 敵
    EnemySystem  enemy_system_;
    // 弾プール: enemy/playerを別プールで管理する
    BulletSystem enemy_bullets_{1200};
    BulletSystem player_bullets_{100};
    // 当たり判定
    CollisionSystem collision_system_;

    // 各段階のUpdate(UpdateTick()から順番に呼ばれる)
    void UpdatePlayer(float dt, const InputState& input);
    PlayerWeaponEvents UpdateWeapons(const InputState& input);
    void UpdateEnemy(float dt);
    void UpdateProjectiles(float dt);
    void ResolveCollisions(bool& player_hit_out, EnemyHitEvents& enemy_hit_out);

public:
    // 初期化
    void Initialize(float width, float height);
    // 固定タイムステップ更新
    GameplayTickEvents UpdateTick(float dt, const InputState& input);
    // 描画(NOTE: RenderPipelineへ移行するまでの暫定実装)
    void Render(PrimitiveRenderer& prim) const;

    // ── デバッグ/統計用 getter ───────────────────────────
    const BulletSystem& GetPlayerBullets() const noexcept{
        return player_bullets_;
    }
    const PlayerWeaponSystem& GetPlayerWeaponSystem() const noexcept{
        return player_weapon_system_;
    }
    const BulletSystem& GetEnemyBullets() const noexcept{
        return enemy_bullets_;
    }
    const CollisionSystem& GetCollisionSystem() const noexcept{
        return collision_system_;
    }

    // ── 実装・性能試験用 ───────────────────────────────────────
    // NOTE: 将来的にはEnemySystemに移行する
    void InitializeBulletStressTest();
    void SpawnEnemyTest();
};

}   // namespace zimovka

#endif  // ZIMOVKA_ENGINE_UPDATE_UPDATEPIPELINE_HPP_
