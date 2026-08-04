#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/enemy/EnemyDamageResult.hpp"
#include "zimovka/systems/enemy/EnemySpawnParams.hpp"
#include "zimovka/systems/enemy/EnemySystem.hpp"

using zimovka::EnemyDamageResult;
using zimovka::EnemySpawnParams;
using zimovka::EnemySystem;
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
