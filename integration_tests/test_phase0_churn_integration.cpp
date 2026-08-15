#include <gtest/gtest.h>

#include <algorithm>
#include <array>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/events/EnemyHitEvents.hpp"
#include "zimovka/systems/bullet/Bullet.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/collision/CollisionSystem.hpp"
#include "zimovka/systems/enemy/Enemy.hpp"
#include "zimovka/systems/enemy/EnemySpawnParams.hpp"
#include "zimovka/systems/enemy/EnemySystem.hpp"

using zimovka::Bullet;
using zimovka::BulletSystem;
using zimovka::CollisionSystem;
using zimovka::Enemy;
using zimovka::EnemyHitEvents;
using zimovka::EnemySpawnParams;
using zimovka::EnemySystem;
using zimovka::Vec2;

namespace {

/**
 * @brief 敵出現用ヘルパ関数
 *
 * @param pos     中心座標
 * @param hp      HP(衝突負荷試験では1を指定)
 */
EnemySpawnParams MakeEnemyParams(Vec2 pos, int hp = 1){
    EnemySpawnParams p;
    p.position       = pos;
    p.velocity       = {0.0f, 0.0f};
    p.render_size    = {32.0f, 32.0f};
    p.hurtbox_offset = {0.0f, 0.0f};
    p.hurtbox_radius = 13.0f;
    p.contact_offset = {0.0f, 0.0f};
    p.contact_radius = 10.0f;
    p.hp             = hp;
    return p;
}

/**
 * @brief active_count_と実際のactiveな要素数を突き合わせるヘルパ
 *
 * CountActive()が管理する内部カウンタと，配列を線形走査して得た実数を比較し
 * active_count_の整合性をTickまたはサイクルをまたいで継続的に確認する
 *
 * @tparam Range  Bullet/Enemyなどactiveフィールドを持つ要素の列
 */
template <class Range>
std::size_t CountActiveByScan(const Range& values){
    return static_cast<std::size_t>(
        std::count_if(
            values.begin(),
            values.end(),
            [](const auto& v){ return v.active; }
        )
    );
}

}   // namespace

// ──────────────────────────────────────────────────────
// 衝突負荷試験(Collision Churn)
// ──────────────────────────────────────────────────────
/**
 * @brief 1000サイクルSpawn→衝突解決→inactiveを繰り返して安定性を確認
 *
 * 配置(毎サイクル):
 *   Player Bullet: 10発 / x=50,130,...,770 y=100 radius=3 velocity=0
 *   Enemy:         10体 / x=50,130,...,770 y=100 (弾と同座標 → 必ず衝突)
 *                  hp=1 → 1ヒットで撃破
 *
 * 衝突判定カウント(1サイクルの期待値):
 *   弾kはスロット0..k-1の敵がinactiveでスキップされスロットkにのみ命中
 *   → player_bullet_vs_enemy_checks = 1+1+...+1 = 10
 *
 * 全サイクル不変条件:
 *   hits.hit_count  == 10
 *   hits.kill_count == 10
 *   enemies.CountActive()        == 0 (衝突後)
 *   player_bullets.CountActive() == 0 (衝突後)
 *   active_count_が実際のactive数と一致(CountActiveByScanで検証)
 *   空きスロットが再利用できる(毎サイクルSpawnが成功)
 *   メモリアドレスが変化しない(ヒープ再確保なし)
 */
TEST(CollisionChurnTest, SpawnCollideRespawnIsStableFor1000Cycles){
    // 容量10で構築(各サイクルで使い切る)
    BulletSystem    player_bullets(10);
    EnemySystem     enemies(10);
    CollisionSystem collision_system;

    // ループ開始前のメモリアドレス(再確保が起きないことの検証に使用)
    const Bullet* bullet_data = player_bullets.GetBullets().data();
    const Enemy*  enemy_data  = enemies.GetEnemies().data();

    for(std::size_t cycle = 0; cycle < 1000; ++cycle){

        // サイクル開始時は全てinactive
        ASSERT_EQ(enemies.CountActive(),        0u) << "cycle=" << cycle;
        ASSERT_EQ(player_bullets.CountActive(), 0u) << "cycle=" << cycle;

        // 敵/自機弾出現処理(弾と敵を同座標に配置するので衝突が確定)
        for(std::size_t i = 0; i < 10; ++i){
            const Vec2 position{
                50.0f + static_cast<float>(i) * 80.0f,
                100.0f
            };
            ASSERT_TRUE(enemies.Spawn(MakeEnemyParams(position)))
                << "cycle=" << cycle << " i=" << i;
            ASSERT_TRUE(player_bullets.Spawn(position, {0.0f, 0.0f}, 3.0f))
                << "cycle=" << cycle << " i=" << i;
        }

        // Spawn後はactive数が正確か確認(Spawn直後は衝突判定をしていない)
        ASSERT_EQ(enemies.CountActive(),        10u) << "cycle=" << cycle;
        ASSERT_EQ(player_bullets.CountActive(), 10u) << "cycle=" << cycle;
        // active数のカウントとの照合もしておく
        ASSERT_EQ(enemies.CountActive(), CountActiveByScan(enemies.GetEnemies()));
        ASSERT_EQ(player_bullets.CountActive(), CountActiveByScan(player_bullets.GetBullets()));

        collision_system.InitializeStatsAtBeginTick();

        // ── 自機弾vs敵の衝突解決 ──
        const EnemyHitEvents hits =
            collision_system.ResolvePlayerBulletsVsEnemies(player_bullets, enemies);

        // hit/kill検証
        ASSERT_EQ(hits.hit_count,  10u) << "cycle=" << cycle;
        ASSERT_EQ(hits.kill_count, 10u) << "cycle=" << cycle;

        // 衝突後は全員inactive
        ASSERT_EQ(enemies.CountActive(),        0u) << "cycle=" << cycle;
        ASSERT_EQ(player_bullets.CountActive(), 0u) << "cycle=" << cycle;

        // active_count_と実際のactive要素数が一致しているかチェック
        ASSERT_EQ(
            enemies.CountActive(),
            CountActiveByScan(enemies.GetEnemies())
        ) << "cycle=" << cycle;
        ASSERT_EQ(
            player_bullets.CountActive(),
            CountActiveByScan(player_bullets.GetBullets())
        ) << "cycle=" << cycle;

        // 衝突判定カウントの検証
        // 弾kはスロット0..k-1の敵がinactiveでスキップ→スロットkのみ++checks
        // → 10発 × 1回 = 10
        ASSERT_EQ(
            collision_system.GetStats().player_bullet_vs_enemy_checks,
            10u
        ) << "cycle=" << cycle;
    }

    // ループ終了後にメモリアドレスが変化していないことを確認(ヒープ再確保なし)
    EXPECT_EQ(player_bullets.GetBullets().data(), bullet_data);
    EXPECT_EQ(enemies.GetEnemies().data(),        enemy_data);
}

/**
 * @brief 最悪計算想定の探索テスト
 * 
 * SpawnCollideRespawnIsStableFor1000Cycles
 * は最良計算に近いので，逆順で弾を配置して判定回数を増加させる
 * ※全弾命中する場合の中で判定回数が多い配置
 * 
 * Bullet 0 → Enemy 9
 * Bullet 1 → Enemy 8
 * ...
 * Bullet 9 → Enemy 0
 * 
 * この場合は判定回数は10+9+...+1=55になる
 */
TEST(CollisionChurnTest, ReverseTargetOrderProduces55Checks){
    // 容量10で構築(各サイクルで使い切る)
    BulletSystem bullets{10};
    EnemySystem enemies{10};
    CollisionSystem collisions;

    std::array<Vec2, 10> positions{};
    // 敵配置
    for (std::size_t i = 0; i < 10; ++i) {
        positions[i] = {
            50.0f + static_cast<float>(i) * 80.0f,
            100.0f
        };
        ASSERT_TRUE(enemies.Spawn(MakeEnemyParams(positions[i])));
    }
    // 弾配置
    for (std::size_t i = 0; i < 10; ++i) {
        ASSERT_TRUE(
            bullets.Spawn(
                positions[9 - i],
                {0.0f, 0.0f},
                3.0f
            )
        );
    }
 
    collisions.InitializeStatsAtBeginTick();
    // ── 自機弾vs敵の衝突解決 ──
    const auto hits =
        collisions.ResolvePlayerBulletsVsEnemies(
            bullets,
            enemies
        );

    EXPECT_EQ(hits.hit_count, 10u);
    EXPECT_EQ(hits.kill_count, 10u);

    EXPECT_EQ(
        collisions.GetStats()
            .player_bullet_vs_enemy_checks,
        55u
    );
}

/**
 * @brief 全弾外れるパターン: 弾10発と敵10体が衝突しないことを確認
 *
 * 配置:
 *   Enemy:         10体 / x=50,130,...,770, y=100 (hurtbox_radius=13)
 *   Player Bullet: 10発 / x=50,130,...,770, y=600 (radius=3, velocity=0)
 *                  → 縦方向の距離 500px >> hurtbox_radius+radius=16 → 衝突なし
 *
 * 期待値:
 *   hits.hit_count  == 0
 *   hits.kill_count == 0
 *   enemies.CountActive()        == 10 (inactiveにならない)
 *   player_bullets.CountActive() == 10 (inactiveにならない)
 *   player_bullet_vs_enemy_checks == 100 (全件走査: 10発×10体)
 */
TEST(CollisionChurnTest, AllBulletsMiss_FullScan100Checks){
    BulletSystem    player_bullets(10);
    EnemySystem     enemies(10);
    CollisionSystem collision_system;

    // 敵: y=100, x=50,130,...,770
    for(std::size_t i = 0; i < 10; ++i){
        ASSERT_TRUE(
            enemies.Spawn(MakeEnemyParams({50.0f + static_cast<float>(i) * 80.0f, 100.0f}))
        ) << "enemy Spawn失敗: i=" << i;
    }
    // 自機弾: y=600(敵とは500px離れており衝突しない), x=50,130,...,770
    for(std::size_t i = 0; i < 10; ++i){
        ASSERT_TRUE(
            player_bullets.Spawn(
                {50.0f + static_cast<float>(i) * 80.0f, 600.0f},
                {0.0f, 0.0f},
                3.0f
            )
        ) << "bullet Spawn失敗: i=" << i;
    }

    ASSERT_EQ(enemies.CountActive(),        10u);
    ASSERT_EQ(player_bullets.CountActive(), 10u);

    collision_system.InitializeStatsAtBeginTick();
    const EnemyHitEvents hits =
        collision_system.ResolvePlayerBulletsVsEnemies(player_bullets, enemies);

    // 全弾外れる
    EXPECT_EQ(hits.hit_count,  0u);
    EXPECT_EQ(hits.kill_count, 0u);

    // 全オブジェクトがactiveのまま
    EXPECT_EQ(enemies.CountActive(),        10u);
    EXPECT_EQ(player_bullets.CountActive(), 10u);

    // 10発 × 10体 = 100回の全件走査が行われる
    EXPECT_EQ(
        collision_system.GetStats().player_bullet_vs_enemy_checks,
        100u
    );
}
