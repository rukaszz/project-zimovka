#include <gtest/gtest.h>

#include "zimovka/core/Circle.hpp"
#include "zimovka/core/Vec2.hpp"
#include "zimovka/events/EnemyHitEvents.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/collision/CollisionSystem.hpp"
#include "zimovka/systems/collision/CollisionUtilities.hpp"
#include "zimovka/systems/enemy/EnemySpawnParams.hpp"
#include "zimovka/systems/enemy/EnemySystem.hpp"
#include "zimovka/systems/player/Player.hpp"

using zimovka::BulletSystem;
using zimovka::Circle;
using zimovka::CollisionSystem;
using zimovka::EnemySpawnParams;
using zimovka::EnemySystem;
using zimovka::Player;
using zimovka::Vec2;

// テスト用の最低限有効なEnemySpawnParams
static EnemySpawnParams MakeEnemyParams(Vec2 position, float hurtbox_radius = 13.0f, int hp = 1){
    EnemySpawnParams p;
    p.position       = position;        // 中心座標は設定可能
    p.velocity       = {0.0f, 0.0f};
    p.render_size    = {32.0f, 32.0f};
    p.hurtbox_offset = {0.0f, 0.0f};
    p.hurtbox_radius = hurtbox_radius;  // 当たり判定は設定可能
    p.contact_offset = {0.0f, 0.0f};
    p.contact_radius = 10.0f;
    p.hp             = hp;              // hpは設定可能
    return p;
}

// ──────────────────────────────────────────────────────
// CollisionUtilities::Intersectsのテスト
// ──────────────────────────────────────────────────────
/**
 * @brief 円同士が重なっているパターン
 * 
 */
TEST(IntersectsTest, Overlapping){
    // 中心(0, 0)の半径5の円
    Circle a{{0.0f, 0.0f}, 5.0f};
    // 中心(3, 0)の半径5の円
    Circle b{{3.0f, 0.0f}, 5.0f};
    // a, bは重なっている
    EXPECT_TRUE(zimovka::CollisionUtilities::Intersects(a, b));
}

/**
 * @brief 円同士が接触状態のパターン
 * 
 */
TEST(IntersectsTest, Touching){
    // 中心距離 == r1 + r2 
    // 10 == 5 + 5 ← 接触 = 被弾
    Circle a{{0.0f, 0.0f}, 5.0f};
    Circle b{{10.0f, 0.0f}, 5.0f};
    // 接触も被弾となる
    EXPECT_TRUE(zimovka::CollisionUtilities::Intersects(a, b));
}

/**
 * @brief 円同士が接触していないパターン
 * 
 */
TEST(IntersectsTest, Separated){
    Circle a{{0.0f, 0.0f}, 3.0f};
    Circle b{{100.0f, 0.0f}, 3.0f};
    EXPECT_FALSE(zimovka::CollisionUtilities::Intersects(a, b));
}

/**
 * @brief 同じ中心座標，同じ半径の2つの円のパターン
 * 
 */
TEST(IntersectsTest, SameCenter){
    Circle a{{5.0f, 5.0f}, 1.0f};
    Circle b{{5.0f, 5.0f}, 1.0f};
    EXPECT_TRUE(zimovka::CollisionUtilities::Intersects(a, b));
}

/**
 * @brief わずかに重なっていないパターン
 * 
 */
TEST(IntersectsTest, JustOutside){
    // 中心距離がr1 + r2をわずかに超える
    Circle a{{0.0f, 0.0f}, 5.0f};
    Circle b{{10.001f, 0.0f}, 5.0f};
    EXPECT_FALSE(zimovka::CollisionUtilities::Intersects(a, b));
}

/**
 * @brief 半径0.0fの円同士のパターン
 * 
 */
TEST(IntersectsTest, ZeroRadiusPoint){
    // 半径0の点の場合，同一座標なら接触
    Circle a{{3.0f, 3.0f}, 0.0f};
    Circle b{{3.0f, 3.0f}, 0.0f};
    EXPECT_TRUE(zimovka::CollisionUtilities::Intersects(a, b));
}

// ──────────────────────────────────────────────────────
// CollisionSystem のテスト
// ──────────────────────────────────────────────────────
/**
 * @brief Playerと非活性の弾との当たり判定
 * 
 */
TEST(CollisionSystemTest, NoHit_EmptyBullets){
    CollisionSystem cs;
    BulletSystem    bs(10);
    Player player;
    player.position = {480.0f, 360.0f};
    // 非活性の弾はないため当たらない
    cs.InitializeStatsAtBeginTick();
    EXPECT_FALSE(cs.CheckPlayerHitByBullets(player, bs));
    // 全て非活性なので走査なし
    EXPECT_EQ(cs.GetStats().player_vs_enemy_bullet_checks, 0u);
}

/**
 * @brief プレイヤーと弾が重なった状態で当たる
 * 
 */
TEST(CollisionSystemTest, Hit_BulletOnPlayer){
    CollisionSystem cs;
    BulletSystem    bs(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 10.0f;
    // プレイヤーの中心に弾を置く
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f);
    EXPECT_TRUE(cs.CheckPlayerHitByBullets(player, bs));
}

/**
 * @brief Playerと弾が離れており当たらない
 * 
 */
TEST(CollisionSystemTest, NoHit_BulletFarAway){
    CollisionSystem cs;
    BulletSystem    bs(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;
    bs.Spawn({900.0f, 600.0f}, {0.0f, 0.0f}, 3.0f);
    // Player中心座標(100, 100)に対して弾の中心座標(900, 600)
    EXPECT_FALSE(cs.CheckPlayerHitByBullets(player, bs));
}

/**
 * @brief CollisionStats::player_vs_enemy_bullet_checksがactiveな弾の数だけ増えることを確認
 *
 */
TEST(CollisionSystemTest, Stats_TracksActiveChecks){
    CollisionSystem cs;
    BulletSystem    bs(5);
    Player player;
    player.position   = {0.0f, 0.0f};
    player.hit_radius = 1.0f;
    // 遠くに3発(どれも当たらない)
    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({901.0f, 901.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({902.0f, 902.0f}, {0.0f, 0.0f}, 1.0f);
    cs.InitializeStatsAtBeginTick();
    cs.CheckPlayerHitByBullets(player, bs);
    // 活性の弾が3つチェックされたので3
    EXPECT_EQ(cs.GetStats().player_vs_enemy_bullet_checks, 3u);
}

/**
 * @brief InitializeStatsAtBeginTick()でCollisionStatsがリセットされることを確認
 *
 */
TEST(CollisionSystemTest, Stats_ResetByInitializeAtBeginTick){
    CollisionSystem cs;
    BulletSystem    bs(5);
    Player player;
    player.position   = {0.0f, 0.0f};
    player.hit_radius = 1.0f;
    // 活性状態の弾2つ(当たらない)
    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({901.0f, 901.0f}, {0.0f, 0.0f}, 1.0f);
    cs.InitializeStatsAtBeginTick();
    cs.CheckPlayerHitByBullets(player, bs);
    EXPECT_EQ(cs.GetStats().player_vs_enemy_bullet_checks, 2u);
    // 弾を消去(非活性化)してStatsをリセット
    bs.Clear();
    cs.InitializeStatsAtBeginTick();
    cs.CheckPlayerHitByBullets(player, bs);
    EXPECT_EQ(cs.GetStats().player_vs_enemy_bullet_checks, 0u); // Clear()で非活性化したのでゼロになる
}

// ──────────────────────────────────────────────────────
// 全件走査と早期return
// ──────────────────────────────────────────────────────
/**
 * @brief 最初の弾がヒットした場合に早期returnし残りを走査しないことを確認
 *
 * スロット0でヒット → スロット1, 2の走査がスキップされる
 */
TEST(CollisionSystemTest, EarlyReturn_FirstBulletHits){
    CollisionSystem cs;
    BulletSystem    bs(5);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 10.0f;

    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f); // スロット0：Playerに当たる
    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f); // スロット1：当たらない(走査されない)
    bs.Spawn({800.0f, 800.0f}, {0.0f, 0.0f}, 1.0f); // スロット2：当たらない(走査されない)

    cs.InitializeStatsAtBeginTick();
    EXPECT_TRUE(cs.CheckPlayerHitByBullets(player, bs));
    // スロット 0 でヒット → early return → count == 1
    EXPECT_EQ(cs.GetStats().player_vs_enemy_bullet_checks, 1u);
}

/**
 * @brief 途中の弾がヒットした場合に以降をスキップすることを確認
 *
 * スロット0(miss) → スロット1(HIT)で早期return → スロット2は走査されない
 */
TEST(CollisionSystemTest, EarlyReturn_MiddleBulletHits){
    CollisionSystem cs;
    BulletSystem    bs(5);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 10.0f;

    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f); // スロット0：miss
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f); // スロット1：HIT
    bs.Spawn({800.0f, 800.0f}, {0.0f, 0.0f}, 1.0f); // スロット2：miss(走査されない)

    cs.InitializeStatsAtBeginTick();
    EXPECT_TRUE(cs.CheckPlayerHitByBullets(player, bs));
    // スロット0(miss→+1)+スロット1(hit→+1) → count=2, スロット2はスキップされる
    EXPECT_EQ(cs.GetStats().player_vs_enemy_bullet_checks, 2u);
}

/**
 * @brief ヒットなしの場合はactiveな弾を全件走査することを確認
 * 
 */
TEST(CollisionSystemTest, FullScan_NoHit_AllActiveChecked){
    CollisionSystem cs;
    BulletSystem    bs(5);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;
    // プレイヤーに当たらない弾3つSpawn
    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({800.0f, 800.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({700.0f, 700.0f}, {0.0f, 0.0f}, 1.0f);
    // 当たらない
    cs.InitializeStatsAtBeginTick();
    EXPECT_FALSE(cs.CheckPlayerHitByBullets(player, bs));
    EXPECT_EQ(cs.GetStats().player_vs_enemy_bullet_checks, 3u); // 3発を全件走査
}

/**
 * @brief inactiveな弾がplayer_vs_enemy_bullet_checksにカウントされないことを確認
 *
 */
TEST(CollisionSystemTest, InactiveBullets_NotCounted){
    CollisionSystem cs;
    BulletSystem    bs(5);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;

    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({800.0f, 800.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Clear(); // 全てinactiveへ

    cs.InitializeStatsAtBeginTick();
    cs.CheckPlayerHitByBullets(player, bs);
    // inactiveな弾はskipされるのでカウントは0
    EXPECT_EQ(cs.GetStats().player_vs_enemy_bullet_checks, 0u);
}

// ──────────────────────────────────────────────────────
// ResolvePlayerBulletsVsEnemies
// ──────────────────────────────────────────────────────
/**
 * @brief 弾が0発のときEnemyHitEventsがゼロであることを確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_NoBullets){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    // PlayerBulletはSpawnしない
    es.Spawn(MakeEnemyParams({100.0f, 100.0f}));

    const auto result = cs.ResolvePlayerBulletsVsEnemies(bs, es);
    EXPECT_EQ(result.hit_count,  0u);
    EXPECT_EQ(result.kill_count, 0u);
}

/**
 * @brief 敵が0体のときEnemyHitEventsがゼロであることを確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_NoEnemies){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f);
    // EnemyはSpawnしない

    const auto result = cs.ResolvePlayerBulletsVsEnemies(bs, es);
    EXPECT_EQ(result.hit_count,  0u);
    EXPECT_EQ(result.kill_count, 0u);
}

/**
 * @brief 弾が敵のhurtboxに当たるとhit_count==1になることを確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_Hit_CountsHit){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    // 敵(中心100,100 hurtbox_radius=13)
    es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 2)); // hp=2→撃破されない
    // 弾(中心100,100 radius=5)→完全に重なりヒット
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f);

    const auto result = cs.ResolvePlayerBulletsVsEnemies(bs, es);
    EXPECT_EQ(result.hit_count,  1u);
    EXPECT_EQ(result.kill_count, 0u); // hp=2なので撃破されない
}

/**
 * @brief ヒット後に自機弾がinactiveになることを確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_Hit_BulletDeactivated){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 2));
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f);

    cs.ResolvePlayerBulletsVsEnemies(bs, es);
    EXPECT_EQ(bs.CountActive(), 0u); // ヒット後にPlayerBulletが非活性化
}

/**
 * @brief hp=1の敵に当たるとkill_count==1になりinactiveになることを確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_Kill_CountsKill){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 1)); // hp=1
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f);

    const auto result = cs.ResolvePlayerBulletsVsEnemies(bs, es);
    EXPECT_EQ(result.hit_count,  1u);
    EXPECT_EQ(result.kill_count, 1u);
    EXPECT_EQ(es.CountActive(), 0u); // 敵が撃破された
}

/**
 * @brief 離れた位置の弾と敵はヒットしないことを確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_NoHit_BulletFarAway){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f));
    bs.Spawn({900.0f, 600.0f}, {0.0f, 0.0f}, 5.0f); // 遠く離れた弾

    const auto result = cs.ResolvePlayerBulletsVsEnemies(bs, es);
    EXPECT_EQ(result.hit_count, 0u);
    EXPECT_EQ(bs.CountActive(), 1u); // 弾はinactiveにならない
}

/**
 * @brief 1発の弾が最初にヒットした敵で止まること(貫通しない)を確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_BulletHitsOnce){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    // 2体の敵が同じ位置に重なる
    es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 2)); // 敵A hp=2
    es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 2)); // 敵B hp=2
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f);

    const auto result = cs.ResolvePlayerBulletsVsEnemies(bs, es);
    // 1発の弾は1体にのみヒット(貫通なし)
    EXPECT_EQ(result.hit_count, 1u);
    EXPECT_EQ(bs.CountActive(), 0u); // 弾は消える
    EXPECT_EQ(es.CountActive(), 2u); // hp=2なのでどちらも生存
}

/**
 * @brief inactiveな弾はヒット判定されないことを確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_InactiveBullets_Skipped){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f));
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f);
    bs.Clear(); // 出現した弾をinactive

    const auto result = cs.ResolvePlayerBulletsVsEnemies(bs, es);   // 弾はinactive
    EXPECT_EQ(result.hit_count, 0u);
    EXPECT_EQ(es.CountActive(), 1u); // 敵は生存
}

/**
 * @brief 2発の弾が2体の敵にそれぞれヒットしhit=2, kill=2になることを確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_MultipleBulletsKillMultipleEnemies){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    // 2体の敵(離れた位置に配置)
    es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 1)); // 敵A
    es.Spawn(MakeEnemyParams({500.0f, 500.0f}, 13.0f, 1)); // 敵B
    // 各敵に対応した弾
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f); // 弾A→敵Aにヒット
    bs.Spawn({500.0f, 500.0f}, {0.0f, 0.0f}, 5.0f); // 弾B→敵Bにヒット

    const auto result = cs.ResolvePlayerBulletsVsEnemies(bs, es);
    EXPECT_EQ(result.hit_count,  2u);
    EXPECT_EQ(result.kill_count, 2u);
    EXPECT_EQ(bs.CountActive(), 0u);
    EXPECT_EQ(es.CountActive(), 0u);
}

/**
 * @brief CollisionStatsのチェック
 * player_bullet_vs_enemy_checksが正しく増えることを確認
 *
 */
TEST(CollisionSystemTest, ResolvePlayerBulletsVsEnemies_Stats_ChecksIncremented){
    CollisionSystem cs;
    BulletSystem    bs(10);
    EnemySystem     es(10);

    cs.InitializeStatsAtBeginTick();
    // 2体の敵 + 1発の弾(当たらない)
    es.Spawn(MakeEnemyParams({500.0f, 500.0f}, 13.0f));
    es.Spawn(MakeEnemyParams({600.0f, 600.0f}, 13.0f));
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f); // 遠くて当たらない
    cs.ResolvePlayerBulletsVsEnemies(bs, es);
    
    // 1発の弾 × 2体の敵 = 2回チェック
    EXPECT_EQ(cs.GetStats().player_bullet_vs_enemy_checks, 2u);
}
