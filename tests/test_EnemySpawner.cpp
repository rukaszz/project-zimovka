#include <gtest/gtest.h>

#include <array>
#include <stdexcept>

#include "zimovka/systems/spawn/EnemySpawner.hpp"

using zimovka::EnemySpawnEvent;
using zimovka::EnemySpawnParams;
using zimovka::EnemySpawnTickResult;
using zimovka::EnemySpawner;
using zimovka::EnemySystem;

namespace{
// テスト用スポーンパラメータ作成関数
// render_size/hurtbox_radius/contact_radius/fire_interval_ticksはデフォルト値
EnemySpawnParams MakeParams(float x = 100.0f, float y = 80.0f){
    EnemySpawnParams p;
    p.position = {x, y};
    return p;
}

} // namespace

/**
 * @brief 指定Tickに達したときにスポーンするか
 * 
 * event tickの前では0体，ちょうどそのtickで1体スポーン
 * ※このテストでIsFinished()のチェックする
 */
TEST(EnemySpawnerTest, BeforeTick_DoesNotSpawn_ExactTickSpawns){
    // 1体出現させるイベント
    const std::array<EnemySpawnEvent, 1> events{{{.tick = 10, .params = MakeParams()}}};
    EnemySystem  enemies{1};
    EnemySpawner spawner{events};

    // tick=9: まだイベント時刻に達していない→スポーンなし
    EnemySpawnTickResult r = spawner.UpdateTick(9, enemies);
    EXPECT_EQ(r.spawned, 0u);
    EXPECT_EQ(r.spawn_failed, 0u);
    EXPECT_EQ(enemies.CountActive(), 0u);
    EXPECT_FALSE(spawner.IsFinished());

    // tick=10: イベント時刻→1体スポーン + IsFinishedチェック
    r = spawner.UpdateTick(10, enemies);
    EXPECT_EQ(r.spawned, 1u);
    EXPECT_EQ(r.spawn_failed, 0u);
    EXPECT_EQ(enemies.CountActive(), 1u);
    EXPECT_TRUE(spawner.IsFinished());
}

/**
 * @brief 同一Tickでの出現イベントが適切に処理されるか
 * 
 */
TEST(EnemySpawnerTest, MultipleEventsSameTick_SpawnInOrder){
    // 2体同時に出現させる
    const std::array<EnemySpawnEvent, 2> events{{
        {.tick = 5, .params = MakeParams(100.0f)},
        {.tick = 5, .params = MakeParams(200.0f)},
    }};
    EnemySystem  enemies{2};
    EnemySpawner spawner{events};
    // 2体出現
    const EnemySpawnTickResult r = spawner.UpdateTick(5, enemies);
    EXPECT_EQ(r.spawned, 2u);
    EXPECT_EQ(r.spawn_failed, 0u);
    EXPECT_EQ(enemies.CountActive(), 2u);
    EXPECT_TRUE(spawner.IsFinished());

    // 配列順確認: 最初のイベントがスロット0, 次がスロット1
    const auto all = enemies.GetEnemies();
    EXPECT_FLOAT_EQ(all[0].position.x, 100.0f);
    EXPECT_FLOAT_EQ(all[1].position.x, 200.0f);
}

/**
 * @brief リセット→再生成がうまくいくか
 * 
 * 一度進めた後Reset()でindex=0に戻り，再度最初から処理可能
 */
TEST(EnemySpawnerTest, Reset_ReplaysFromFirstEvent){
    const std::array<EnemySpawnEvent, 1> events{{{.tick = 10, .params = MakeParams()}}};
    EnemySystem  enemies{2};   // リセット後の再スポーン用に2スロット確保
    EnemySpawner spawner{events};

    // 一度消化
    (void)spawner.UpdateTick(10, enemies);
    EXPECT_TRUE(spawner.IsFinished());
    EXPECT_EQ(spawner.GetNextEventIndex(), 1u);

    // Reset: インデックスが0に戻る
    spawner.Reset();
    EXPECT_EQ(spawner.GetNextEventIndex(), 0u);
    EXPECT_FALSE(spawner.IsFinished());

    // 再度 tick=10→もう1体スポーン(合計2体)
    const EnemySpawnTickResult r = spawner.UpdateTick(10, enemies);
    EXPECT_EQ(r.spawned, 1u);
    EXPECT_EQ(enemies.CountActive(), 2u);
}

/**
 * @brief プール満杯時の処理確認
 * 
 * failed+1，インデックスが進む
 */
TEST(EnemySpawnerTest, PoolFull_ConsumesEventAndReportsFailure){
    const std::array<EnemySpawnEvent, 2> events{{
        {.tick = 10, .params = MakeParams()},
        {.tick = 20, .params = MakeParams()},
    }};
    EnemySystem  enemies{1};   // capacity=1: tick=10で満杯になる
    EnemySpawner spawner{events};

    // tick=10: 1体スポーン成功
    EnemySpawnTickResult r = spawner.UpdateTick(10, enemies);
    EXPECT_EQ(r.spawned, 1u);
    EXPECT_EQ(r.spawn_failed, 0u);
    EXPECT_EQ(spawner.GetNextEventIndex(), 1u);

    // tick=20: プール満杯 → 失敗, ただしインデックスは進む
    r = spawner.UpdateTick(20, enemies);
    EXPECT_EQ(r.spawned, 0u);
    EXPECT_EQ(r.spawn_failed, 1u);
    EXPECT_EQ(spawner.GetNextEventIndex(), 2u);

    // 全イベント消化済み → IsFinished & 次tickにリトライされない
    EXPECT_TRUE(spawner.IsFinished());
    r = spawner.UpdateTick(21, enemies);
    EXPECT_EQ(r.spawned, 0u);
    EXPECT_EQ(r.spawn_failed, 0u);
}

/**
 * @brief tickが降順のイベント配列を渡すとコンストラクタが例外を投げる
 * 
 */
TEST(EnemySpawnerTest, UnsortedEvents_ConstructorRejects){
    const std::array<EnemySpawnEvent, 2> unsorted{{
        {.tick = 20, .params = MakeParams()},
        {.tick = 10, .params = MakeParams()},  // 降順: 不正
    }};
    EXPECT_THROW(
        (EnemySpawner{unsorted}),
        std::invalid_argument
    );
}
