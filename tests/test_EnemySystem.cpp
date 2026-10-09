#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/enemy/EnemyDamageResult.hpp"
#include "zimovka/systems/enemy/EnemySpawnParams.hpp"
#include "zimovka/systems/enemy/EnemySystem.hpp"

using zimovka::BulletSystem;
using zimovka::EnemyAttackPattern;
using zimovka::EnemyDamageResult;
using zimovka::EnemySpawnParams;
using zimovka::EnemySystem;
using zimovka::PatternSystem;
using zimovka::Vec2;

// 有効なSpawnParams(全テストで利用するので静的に定義)
static EnemySpawnParams MakeDefaultParams(Vec2 position = {100.0f, 100.0f}){
    EnemySpawnParams p;
    p.position       = position;    // 中心座標だけは設定可能
    p.velocity       = {0.0f, 0.0f};
    p.render_size    = {32.0f, 32.0f};
    p.hurtbox_offset = {0.0f, 0.0f};
    p.hurtbox_radius = 13.0f;
    p.contact_offset = {0.0f, 0.0f};
    p.contact_radius = 10.0f;
    p.hp             = 1;
    return p;
}

// ──────────────────────────────────────────────────────
// 初期状態
// ──────────────────────────────────────────────────────
/**
 * @brief EnemySystemの初期状態確認
 *
 */
TEST(EnemySystemTest, InitialState){
    EnemySystem es(10);
    EXPECT_EQ(es.CountActive(), 0u);
    EXPECT_EQ(es.GetCapacity(), 10u);
}

// ──────────────────────────────────────────────────────
// Spawn
// ──────────────────────────────────────────────────────
/**
 * @brief 正常なSpawn()の呼び出し
 *
 */
TEST(EnemySystemTest, SpawnSucceeds){
    EnemySystem es(5);
    EXPECT_TRUE(es.Spawn(MakeDefaultParams()));
    EXPECT_EQ(es.CountActive(), 1u);
}

/**
 * @brief Spawn後のEnemyデータが正しくコピーされていることを確認
 *
 */
TEST(EnemySystemTest, SpawnCopiesParams){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams({200.0f, 300.0f});
    p.hp             = 3;
    p.hurtbox_radius = 20.0f;
    EXPECT_TRUE(es.Spawn(p));
    // コピーして値チェック
    const auto enemies = es.GetEnemies();
    EXPECT_FLOAT_EQ(enemies[0].position.x, 200.0f);
    EXPECT_FLOAT_EQ(enemies[0].position.y, 300.0f);
    EXPECT_EQ(enemies[0].hp, 3);
    EXPECT_FLOAT_EQ(enemies[0].hurtbox_radius, 20.0f);
}

/**
 * @brief プール満杯までSpawnできることを確認
 *
 */
TEST(EnemySystemTest, SpawnFillsPool){
    EnemySystem es(3);
    EXPECT_TRUE(es.Spawn(MakeDefaultParams()));
    EXPECT_TRUE(es.Spawn(MakeDefaultParams()));
    EXPECT_TRUE(es.Spawn(MakeDefaultParams()));
    EXPECT_EQ(es.CountActive(), 3u);
}

/**
 * @brief プール満杯で追加Spawnが失敗することを確認
 *
 */
TEST(EnemySystemTest, SpawnFailsWhenPoolFull){
    EnemySystem es(2);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    EXPECT_FALSE(es.Spawn(MakeDefaultParams()));
    EXPECT_EQ(es.CountActive(), 2u);
}

/**
 * @brief hp=0でSpawnが失敗することを確認
 *
 */
TEST(EnemySystemTest, SpawnInvalidHp_Zero){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    p.hp = 0;
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief hp<0でSpawnが失敗することを確認
 *
 */
TEST(EnemySystemTest, SpawnInvalidHp_Negative){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    p.hp = -1;
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief hurtbox_radius=0でSpawnが失敗することを確認
 *
 */
TEST(EnemySystemTest, SpawnInvalidHurtboxRadius_Zero){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    p.hurtbox_radius = 0.0f;
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief hurtbox_radius<0でSpawnが失敗することを確認
 *
 */
TEST(EnemySystemTest, SpawnInvalidHurtboxRadius_Negative){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    p.hurtbox_radius = -1.0f;
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief render_size.x=0でSpawnが失敗することを確認
 * ※一方が無効な値の場合をチェック
 */
TEST(EnemySystemTest, SpawnInvalidRenderSize_Zero){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    p.render_size = {0.0f, 32.0f};
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief position.x=NaNでSpawnが失敗することを確認
 *
 */
TEST(EnemySystemTest, SpawnInvalidPosition_NaN){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    // 浮動小数点数型のNaN
    p.position.x = std::numeric_limits<float>::quiet_NaN();
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief position.x=+Infでも失敗することを確認
 *
 */
TEST(EnemySystemTest, SpawnInvalidPosition_Inf){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    // 浮動小数点数型の性の無限表現
    p.position.x = std::numeric_limits<float>::infinity();
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief hurtbox_radius=+Infでも失敗することを確認
 *
 */
TEST(EnemySystemTest, SpawnInvalidHurtbox_Inf){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    // 浮動小数点数型の性の無限表現
    p.hurtbox_radius = std::numeric_limits<float>::infinity();
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief contact_radius=+Infでも失敗することを確認
 *
 */
TEST(EnemySystemTest, SpawnInvalidContsct_Inf){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    // 浮動小数点数型の性の無限表現
    p.contact_radius = std::numeric_limits<float>::infinity();
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

// ──────────────────────────────────────────────────────
// Clear
// ──────────────────────────────────────────────────────
/**
 * @brief Clear()でCountActive()がゼロになることを確認
 *
 */
TEST(EnemySystemTest, Clear_ResetsActiveCount){
    EnemySystem es(5);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    es.Clear();
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief Clear()後に再Spawnできることを確認
 *
 */
TEST(EnemySystemTest, Clear_AllowsRespawn){
    EnemySystem es(2);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    es.Clear();
    EXPECT_TRUE(es.Spawn(MakeDefaultParams()));
    EXPECT_EQ(es.CountActive(), 1u);
}

/**
 * @brief Clear()後にGetEnemies()の全要素がinactiveになっていることを確認
 *
 */
TEST(EnemySystemTest, Clear_AllEnemiesInactive){
    EnemySystem es(3);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    es.Clear();
    for(const auto& e : es.GetEnemies()){
        EXPECT_FALSE(e.active);
    }
}

// ──────────────────────────────────────────────────────
// Update: 移動と画面外消去
// ──────────────────────────────────────────────────────
/**
 * @brief Update()で座標が速度分だけ移動することを確認
 *
 */
TEST(EnemySystemTest, Update_MovesEnemy){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams({0.0f, 0.0f});
    p.velocity = {60.0f, 0.0f};
    ASSERT_TRUE(es.Spawn(p));
    es.Update((1.0f/60.0f), 960.0f, 720.0f);
    const auto enemies = es.GetEnemies();
    // 60.0f/60.0fで1.0fくらい進むはず
    EXPECT_NEAR(enemies[0].position.x, 1.0f, 1e-4f);
}

/**
 * @brief 画面外へ出た敵がinactiveになることを確認
 *
 */
TEST(EnemySystemTest, Update_RemovesOutOfScreenEnemy){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams({0.0f, 0.0f});
    p.velocity = {-99999.0f, 0.0f}; // 高速で左外へ
    ASSERT_TRUE(es.Spawn(p));
    es.Update(1.0f, 960.0f, 720.0f);
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief 画面内の敵はUpdate()後もactiveのままであることを確認
 *
 */
TEST(EnemySystemTest, Update_KeepsEnemyOnScreen){
    EnemySystem es(5);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams({480.0f, 360.0f})));
    es.Update((1.0f/60.0f), 960.0f, 720.0f);
    EXPECT_EQ(es.CountActive(), 1u);
}

// ──────────────────────────────────────────────────────
// TakeDamage
// ──────────────────────────────────────────────────────
/**
 * @brief hp>1の敵にダメージ1でDamagedが返り，activeのままであることを確認
 *
 */
TEST(EnemySystemTest, TakeDamage_Damaged){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    p.hp = 3;
    ASSERT_TRUE(es.Spawn(p));
    // 1ダメージ
    const EnemyDamageResult result = es.TakeDamage(0, 1);
    EXPECT_EQ(result, EnemyDamageResult::Damaged);
    EXPECT_EQ(es.CountActive(), 1u); // まだ生存
}

/**
 * @brief hp=1の敵にダメージ1でDestroyedが返り，inactiveになることを確認
 *
 */
TEST(EnemySystemTest, TakeDamage_Destroyed){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    p.hp = 1;
    ASSERT_TRUE(es.Spawn(p));
    const EnemyDamageResult result = es.TakeDamage(0, 1);
    EXPECT_EQ(result, EnemyDamageResult::Destroyed);
    EXPECT_EQ(es.CountActive(), 0u); // 撃破
}

/**
 * @brief ダメージがhpを超える場合でもDestroyedが返ることを確認
 *
 */
TEST(EnemySystemTest, TakeDamage_OverkillDestroyed){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    p.hp = 2;
    ASSERT_TRUE(es.Spawn(p));
    const EnemyDamageResult result = es.TakeDamage(0, 99);
    EXPECT_EQ(result, EnemyDamageResult::Destroyed);
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief inactiveな敵へのTakeDamage()でInvalidTargetが返ることを確認
 *
 */
TEST(EnemySystemTest, TakeDamage_InvalidTarget_Inactive){
    EnemySystem es(5);
    // Spawnしていないのでインデックス[0]はinactive
    const EnemyDamageResult result = es.TakeDamage(0, 1);
    EXPECT_EQ(result, EnemyDamageResult::InvalidTarget);
}

/**
 * @brief damage=0でInvalidTargetが返ることを確認
 *
 */
TEST(EnemySystemTest, TakeDamage_InvalidTarget_ZeroDamage){
    EnemySystem es(5);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    const EnemyDamageResult result = es.TakeDamage(0, 0);
    EXPECT_EQ(result, EnemyDamageResult::InvalidTarget);
    EXPECT_EQ(es.CountActive(), 1u); // ダメージなし
}

/**
 * @brief damage<0でInvalidTargetが返ることを確認
 * ※hpが回復しないことを確認
 *
 */
TEST(EnemySystemTest, TakeDamage_InvalidTarget_NegativeDamage){
    EnemySystem es(5);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    const EnemyDamageResult result = es.TakeDamage(0, -1);
    EXPECT_EQ(result, EnemyDamageResult::InvalidTarget);
    EXPECT_EQ(es.CountActive(), 1u); // hpが回復していない
}

/**
 * @brief 範囲外インデックスでstd::out_of_rangeが送出されることを確認
 *
 */
TEST(EnemySystemTest, TakeDamage_OutOfRange_Throws){
    EnemySystem es(5);
    EXPECT_THROW(es.TakeDamage(5, 1), std::out_of_range); // capacity==5, index==5は範囲外
    EXPECT_THROW(es.TakeDamage(100, 1), std::out_of_range);
}

// ──────────────────────────────────────────────────────
// CountActive / GetCapacity
// ──────────────────────────────────────────────────────
/**
 * @brief Spawn/TakeDamage/ClearをまたいでCountActive()が正確に追跡することを確認
 *
 */
TEST(EnemySystemTest, CountActive_TracksMutations){
    EnemySystem es(5);
    EXPECT_EQ(es.CountActive(), 0u);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    EXPECT_EQ(es.CountActive(), 1u);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    EXPECT_EQ(es.CountActive(), 2u);
    es.TakeDamage(0, 999); // 撃破
    EXPECT_EQ(es.CountActive(), 1u);
    es.Clear();
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief GetCapacity()がコンストラクタの引数と一致し変化しないことを確認
 *
 */
TEST(EnemySystemTest, GetCapacity_FixedAfterConstruct){
    EnemySystem es(8);
    EXPECT_EQ(es.GetCapacity(), 8u);
    ASSERT_TRUE(es.Spawn(MakeDefaultParams()));
    es.Clear();
    EXPECT_EQ(es.GetCapacity(), 8u); // Spawn/Clearで変化しない
}

// ──────────────────────────────────────────────────────
// Spawn: EnemyAttackPattern検証
// ──────────────────────────────────────────────────────
/**
 * @brief EnemyAttackPattern::CountでSpawnが失敗するか
 * IsValidEnemySpawnParams()のswitchの門番(Count/default)が機能しているか確認
 * 
 */
TEST(EnemySystemTest, SpawnRejectsCountAttackPattern){
    EnemySystem es(5);
    EnemySpawnParams p = MakeDefaultParams();
    p.attack_pattern = EnemyAttackPattern::Count;
    EXPECT_FALSE(es.Spawn(p));
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief 弾発射パラメータが不正な場合にSpawnが失敗することを確認
 * IsValidEnemySpawnParams()のvalid_fireブランチの代表的な4条件をチェックする
 * 
 */
TEST(EnemySystemTest, SpawnRejectsInvalidFireParameters){
    // fire_bullet_count = 0(弾数ゼロ)
    {
        EnemySystem es(5);
        EnemySpawnParams p = MakeDefaultParams();
        p.fire_bullet_count = 0;
        EXPECT_FALSE(es.Spawn(p));
        EXPECT_EQ(es.CountActive(), 0u);
    }
    // fire_bullet_speed = 0.0f(弾速ゼロ)
    {
        EnemySystem es(5);
        EnemySpawnParams p = MakeDefaultParams();
        p.fire_bullet_speed = 0.0f;
        EXPECT_FALSE(es.Spawn(p));
        EXPECT_EQ(es.CountActive(), 0u);
    }
    // fire_interval_ticks = 0(発射間隔ゼロ → 毎tick発射になり不正)
    {
        EnemySystem es(5);
        EnemySpawnParams p = MakeDefaultParams();
        p.fire_interval_ticks = 0;
        EXPECT_FALSE(es.Spawn(p));
        EXPECT_EQ(es.CountActive(), 0u);
    }
    // fire_spread_rad = -0.1f(負の拡散角度)
    {
        EnemySystem es(5);
        EnemySpawnParams p = MakeDefaultParams();
        p.fire_spread_rad = -0.1f;
        EXPECT_FALSE(es.Spawn(p));
        EXPECT_EQ(es.CountActive(), 0u);
    }
}

// ──────────────────────────────────────────────────────
// UpdateFire: 攻撃パターン検証
// ──────────────────────────────────────────────────────
/**
 * @brief AimedSpreadの敵が自機方向に弾を発射するか
 * 自機を敵の真下に配置し，発射された弾が真下方向(angle≈π/2)を向くかチェック
 * 
 */
TEST(EnemySystemTest, AimedSpread_AimsAtPlayer){
    EnemySystem es(5);
    BulletSystem enemy_bullets(10);
    PatternSystem pattern;

    // 自機の真上に敵を配置 → 期待発射方向 = 真下(π/2)
    EnemySpawnParams p = MakeDefaultParams({320.0f, 100.0f});
    p.attack_pattern           = EnemyAttackPattern::AimedSpread;
    p.fire_bullet_count        = 1;
    p.fire_spread_rad          = 0.0f;      // 単発なのでスプレッドなし
    p.fire_bullet_speed        = 200.0f;
    p.initial_fire_delay_ticks = 0;         // 即時発射
    p.fire_interval_ticks      = 120;
    ASSERT_TRUE(es.Spawn(p));

    // 敵の真下にプレイヤーを配置
    const Vec2 player_pos = {320.0f, 500.0f};
    es.UpdateFire(pattern, enemy_bullets, player_pos);

    ASSERT_EQ(enemy_bullets.CountActive(), 1u);

    // activeな弾の速度を取得
    Vec2 bullet_vel{};
    for(const auto& b : enemy_bullets.GetBullets()){
        if(b.active){
            bullet_vel = b.velocity;
            break;
        }
    }
    // 真下方向(angle=π/2): cos(π/2)≈0, sin(π/2)=1 → velocity ≈ (0, +200)
    EXPECT_NEAR(bullet_vel.x, 0.0f,   1e-3f);
    EXPECT_NEAR(bullet_vel.y, 200.0f, 1e-3f);
}

/**
 * @brief FixedSpreadの敵が自機位置に依存しない弾を発射するか
 * 方向が大きく異なる2つの自機位置で発射し，同一の速度ベクトルが得られることを確認
 * 
 */
TEST(EnemySystemTest, FixedSpread_DoesNotDependOnPlayerPosition){
    const float fixed_angle = std::numbers::pi_v<float> / 2.0f; // π/2→真下

    EnemySpawnParams p = MakeDefaultParams({320.0f, 100.0f});
    p.attack_pattern           = EnemyAttackPattern::FixedSpread;
    p.fixed_fire_angle_rad     = fixed_angle;
    p.fire_bullet_count        = 1;
    p.fire_spread_rad          = 0.0f;
    p.fire_bullet_speed        = 200.0f;
    p.initial_fire_delay_ticks = 0;
    p.fire_interval_ticks      = 120;

    PatternSystem pattern;

    // 自機位置A(左上)
    BulletSystem bullets_A(10);
    {
        EnemySystem es(5);
        ASSERT_TRUE(es.Spawn(p));
        es.UpdateFire(pattern, bullets_A, {0.0f, 0.0f});
    }
    ASSERT_EQ(bullets_A.CountActive(), 1u);

    // 自機位置B(右下)
    BulletSystem bullets_B(10);
    {
        EnemySystem es(5);
        ASSERT_TRUE(es.Spawn(p));
        es.UpdateFire(pattern, bullets_B, {640.0f, 400.0f});
    }
    ASSERT_EQ(bullets_B.CountActive(), 1u);

    // 速度取得
    Vec2 vel_A{}, vel_B{};
    for(const auto& b : bullets_A.GetBullets()){
        if(b.active){
            vel_A = b.velocity;
            break;
        }
    }
    for(const auto& b : bullets_B.GetBullets()){
        if(b.active){
            vel_B = b.velocity;
            break;
        }
    }

    // FixedSpreadなのでプレイヤー位置によらず同一速度
    EXPECT_FLOAT_EQ(vel_A.x, vel_B.x);
    EXPECT_FLOAT_EQ(vel_A.y, vel_B.y);
    // fixed_fire_angle_rad = π/2 (真下) 方向であることも確認
    EXPECT_NEAR(vel_A.x, 0.0f,   1e-3f);
    EXPECT_NEAR(vel_A.y, 200.0f, 1e-3f);
}
