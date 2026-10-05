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
    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f));
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
    ASSERT_TRUE(bs.Spawn({900.0f, 600.0f}, {0.0f, 0.0f}, 3.0f));
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
    ASSERT_TRUE(bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f));
    ASSERT_TRUE(bs.Spawn({901.0f, 901.0f}, {0.0f, 0.0f}, 1.0f));
    ASSERT_TRUE(bs.Spawn({902.0f, 902.0f}, {0.0f, 0.0f}, 1.0f));
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
    ASSERT_TRUE(bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f));
    ASSERT_TRUE(bs.Spawn({901.0f, 901.0f}, {0.0f, 0.0f}, 1.0f));
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

    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f)); // スロット0：Playerに当たる
    ASSERT_TRUE(bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f)); // スロット1：当たらない(走査されない)
    ASSERT_TRUE(bs.Spawn({800.0f, 800.0f}, {0.0f, 0.0f}, 1.0f)); // スロット2：当たらない(走査されない)

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

    ASSERT_TRUE(bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f)); // スロット0：miss
    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f)); // スロット1：HIT
    ASSERT_TRUE(bs.Spawn({800.0f, 800.0f}, {0.0f, 0.0f}, 1.0f)); // スロット2：miss(走査されない)

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
    ASSERT_TRUE(bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f));
    ASSERT_TRUE(bs.Spawn({800.0f, 800.0f}, {0.0f, 0.0f}, 1.0f));
    ASSERT_TRUE(bs.Spawn({700.0f, 700.0f}, {0.0f, 0.0f}, 1.0f));
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

    ASSERT_TRUE(bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f));
    ASSERT_TRUE(bs.Spawn({800.0f, 800.0f}, {0.0f, 0.0f}, 1.0f));
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
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f})));

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

    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f));
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
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 2))); // hp=2→撃破されない
    // 弾(中心100,100 radius=5)→完全に重なりヒット
    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f));

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

    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 2)));
    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f));

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

    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 1))); // hp=1
    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f));

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

    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f)));
    ASSERT_TRUE(bs.Spawn({900.0f, 600.0f}, {0.0f, 0.0f}, 5.0f)); // 遠く離れた弾

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
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 2))); // 敵A hp=2
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 2))); // 敵B hp=2
    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f));

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

    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f)));
    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f));
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
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f}, 13.0f, 1))); // 敵A
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({500.0f, 500.0f}, 13.0f, 1))); // 敵B
    // 各敵に対応した弾
    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f)); // 弾A→敵Aにヒット
    ASSERT_TRUE(bs.Spawn({500.0f, 500.0f}, {0.0f, 0.0f}, 5.0f)); // 弾B→敵Bにヒット

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
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({500.0f, 500.0f}, 13.0f)));
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({600.0f, 600.0f}, 13.0f)));
    ASSERT_TRUE(bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f)); // 遠くて当たらない
    cs.ResolvePlayerBulletsVsEnemies(bs, es);

    // 1発の弾 × 2体の敵 = 2回チェック
    EXPECT_EQ(cs.GetStats().player_bullet_vs_enemy_checks, 2u);
}

// ──────────────────────────────────────────────────────
// 自機弾vs敵弾打ち消し (ResolvePlayerBulletsVsEnemyBullets)
// ──────────────────────────────────────────────────────
/**
 * @brief 重なり合う自機弾と敵弾が互いに非活性化(inactive)されることを確認
 * 
 */
TEST(BulletCancelTest, Overlap_DeactivatesBoth){
    CollisionSystem cs;
    BulletSystem player_bs(10);
    BulletSystem enemy_bs(10);
    // 同一座標へ出現
    ASSERT_TRUE(player_bs.Spawn({100.0f, 100.0f}, {0.0f, -720.0f}, 5.0f));
    ASSERT_TRUE(enemy_bs.Spawn( {100.0f, 100.0f}, {0.0f,  200.0f}, 5.0f));

    const auto result = cs.ResolvePlayerBulletsVsEnemyBullets(player_bs, enemy_bs);
    // 打ち消し数1, Player/Enemy Bulletsのactive数0
    EXPECT_EQ(result.cancel_count, 1u);
    EXPECT_EQ(player_bs.CountActive(), 0u);
    EXPECT_EQ(enemy_bs.CountActive(),  0u);
}

/**
 * @brief 離れた位置の自機弾と敵弾が打ち消されないことを確認
 * 
 */
TEST(BulletCancelTest, NoOverlap_DoesNothing){
    CollisionSystem cs;
    BulletSystem player_bs(10);
    BulletSystem enemy_bs(10);
    // 全く重ならない座標へ出現
    ASSERT_TRUE(player_bs.Spawn({100.0f, 100.0f}, {0.0f, -720.0f}, 5.0f));
    ASSERT_TRUE(enemy_bs.Spawn( {900.0f, 600.0f}, {0.0f,  200.0f}, 5.0f));

    const auto result = cs.ResolvePlayerBulletsVsEnemyBullets(player_bs, enemy_bs);
    // 打ち消し数0, Player/Enemy Bulletsのactive数1
    EXPECT_EQ(result.cancel_count, 0u);
    EXPECT_EQ(player_bs.CountActive(), 1u);
    EXPECT_EQ(enemy_bs.CountActive(),  1u);
}

/**
 * @brief 1発の自機弾が複数の敵弾のうち1発のみを打ち消すことを確認
 *
 * 自機弾と敵弾は1対1で打ち消し合う仕様
 * 打ち消し後に自機弾が非活性化され，残りの敵弾は生存する
 */
TEST(BulletCancelTest, OnePlayerBullet_CancelsOnlyOneEnemyBullet){
    CollisionSystem cs;
    BulletSystem player_bs(10);
    BulletSystem enemy_bs(10);

    ASSERT_TRUE(player_bs.Spawn({100.0f, 100.0f}, {0.0f, -720.0f}, 5.0f));
    ASSERT_TRUE(enemy_bs.Spawn( {100.0f, 100.0f}, {0.0f,  200.0f}, 5.0f)); // スロット0: 打ち消される
    ASSERT_TRUE(enemy_bs.Spawn( {100.0f, 100.0f}, {0.0f,  200.0f}, 5.0f)); // スロット1: 残る
    // 敵弾スロット0→1で判定される
    const auto result = cs.ResolvePlayerBulletsVsEnemyBullets(player_bs, enemy_bs);

    EXPECT_EQ(result.cancel_count, 1u);
    EXPECT_EQ(player_bs.CountActive(), 0u); // 自機弾は非活性化
    EXPECT_EQ(enemy_bs.CountActive(),  1u); // 敵弾スロット1が残る
}

/**
 * @brief 2発の自機弾と1発の敵弾のヒット時に打ち消し回数が1であることを確認
 *
 * 自機弾スロット0が敵弾を打ち消す
 * 自機弾スロット1は活性のままだが対応する敵弾がないため打ち消しは発生しない
 */
TEST(BulletCancelTest, TwoPlayerBullets_OneEnemyBullet_CancelCountIsOne){
    CollisionSystem cs;
    BulletSystem player_bs(10);
    BulletSystem enemy_bs(10);

    ASSERT_TRUE(player_bs.Spawn({100.0f, 100.0f}, {0.0f, -720.0f}, 5.0f)); // スロット0
    ASSERT_TRUE(player_bs.Spawn({100.0f, 100.0f}, {0.0f, -720.0f}, 5.0f)); // スロット1
    ASSERT_TRUE(enemy_bs.Spawn( {100.0f, 100.0f}, {0.0f,  200.0f}, 5.0f));
    // 自機弾スロット0→1で判定される
    const auto result = cs.ResolvePlayerBulletsVsEnemyBullets(player_bs, enemy_bs);

    EXPECT_EQ(result.cancel_count, 1u);
    EXPECT_EQ(player_bs.CountActive(), 1u); // スロット1の自機弾が残る
    EXPECT_EQ(enemy_bs.CountActive(),  0u); // 敵弾は非活性化
}

/**
 * @brief 打ち消しされた敵弾が同Tickのプレイヤー被弾判定に使われないことを確認
 *
 * UpdatePipeline::ResolveCollisions()の処理順序に従い，
 * ResolvePlayerBulletsVsEnemyBullets → CheckPlayerHitByBulletsの処理順序で
 * 打ち消し済みの敵弾(inactive)がプレイヤーに当たらないことをテストする
 */
TEST(BulletCancelTest, CancelledEnemyBullet_CannotHitPlayerSameTick){
    CollisionSystem cs;
    BulletSystem player_bs(10);
    BulletSystem enemy_bs(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 10.0f;

    // プレイヤーと同座標に自機弾と敵弾を配置
    ASSERT_TRUE(player_bs.Spawn({100.0f, 100.0f}, {0.0f, -720.0f}, 5.0f));
    ASSERT_TRUE(enemy_bs.Spawn( {100.0f, 100.0f}, {0.0f,  200.0f}, 5.0f));

    // 1. 打ち消し処理 → 敵弾が非活性化
    const auto cancel = cs.ResolvePlayerBulletsVsEnemyBullets(player_bs, enemy_bs);
    ASSERT_EQ(cancel.cancel_count, 1u);
    ASSERT_EQ(enemy_bs.CountActive(), 0u);

    // 2. プレイヤー被弾判定 → 打ち消された敵弾はinactiveなのでヒットしない
    EXPECT_FALSE(cs.CheckPlayerHitByBullets(player, enemy_bs));
}

// ──────────────────────────────────────────────────────
// プレイヤーvs敵の接触判定 (CheckPlayerHitByEnemies)
// ──────────────────────────────────────────────────────
/**
 * @brief 活性状態の敵とプレイヤーが重なるとtrueを返すことを確認
 * 
 */
TEST(CollisionSystemTest, CheckPlayerHitByEnemies_Hit_ActiveEnemy){
    CollisionSystem cs;
    EnemySystem es(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;

    // プレイヤーの中心に敵を配置(contact_radius=10 → 重なる)
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f})));
    EXPECT_TRUE(cs.CheckPlayerHitByEnemies(player, es));
}

/**
 * @brief 非活性状態の敵はヒット判定されないことを確認
 * 
 */
TEST(CollisionSystemTest, CheckPlayerHitByEnemies_NoHit_InactiveEnemy){
    CollisionSystem cs;
    EnemySystem es(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;

    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f})));
    es.Clear(); // 全て非活性化
    EXPECT_FALSE(cs.CheckPlayerHitByEnemies(player, es));
}

/**
 * @brief 離れた位置の敵はヒット判定されないことを確認
 * 
 */
TEST(CollisionSystemTest, CheckPlayerHitByEnemies_NoHit_EnemyFarAway){
    CollisionSystem cs;
    EnemySystem es(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;

    ASSERT_TRUE(es.Spawn(MakeEnemyParams({900.0f, 600.0f})));
    EXPECT_FALSE(cs.CheckPlayerHitByEnemies(player, es));
}

/**
 * @brief 活性状態の敵の数だけplayer_vs_enemy_checksが増えることを確認
 * 
 */
TEST(CollisionSystemTest, CheckPlayerHitByEnemies_Stats_TracksChecks){
    CollisionSystem cs;
    EnemySystem es(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;

    // 当たらない位置に3体
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({900.0f, 900.0f})));
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({800.0f, 800.0f})));
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({700.0f, 700.0f})));

    cs.InitializeStatsAtBeginTick();    // collision_stats_の初期化
    cs.CheckPlayerHitByEnemies(player, es);
    EXPECT_EQ(cs.GetStats().player_vs_enemy_checks, 3u);
}

/**
 * @brief 最初の敵でヒットした場合に以降をスキップすることを確認
 *
 * スロット0(hit) → early return → スロット1は走査されない
 */
TEST(CollisionSystemTest, CheckPlayerHitByEnemies_EarlyReturn_FirstHit){
    CollisionSystem cs;
    EnemySystem es(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;

    ASSERT_TRUE(es.Spawn(MakeEnemyParams({100.0f, 100.0f}))); // スロット0: hit
    ASSERT_TRUE(es.Spawn(MakeEnemyParams({900.0f, 900.0f}))); // スロット1: 走査されない

    cs.InitializeStatsAtBeginTick();
    EXPECT_TRUE(cs.CheckPlayerHitByEnemies(player, es));
    EXPECT_EQ(cs.GetStats().player_vs_enemy_checks, 1u);
}

/**
 * @brief contact_offsetが接触判定に反映されることを確認
 *
 * 敵の中心座標は(200, 100)だが contact_offset=(-100, 0)のため
 * 接触判定円の中心は(100, 100) → プレイヤーと重なる
 */
TEST(CollisionSystemTest, CheckPlayerHitByEnemies_Hit_WithContactOffset){
    CollisionSystem cs;
    EnemySystem es(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;

    EnemySpawnParams p = MakeEnemyParams({200.0f, 100.0f}); // 中心は(200, 100)
    p.contact_offset  = {-100.0f, 0.0f};                    // 接触判定の半径は(100, 100)
    ASSERT_TRUE(es.Spawn(p));

    // GetContactCircle()が正しく適用されるとヒットになる
    EXPECT_TRUE(cs.CheckPlayerHitByEnemies(player, es));
}

// ──────────────────────────────────────────────────────
// cancel_radius による打ち消し判定範囲のテスト
// ──────────────────────────────────────────────────────
/**
 * @brief cancel_radiusにより通常半径の外側でも打ち消しが発生することを確認
 *
 * 自機弾: radius=5, cancel_radius=15
 * 敵弾:   radius=5(cancel_radiusは通常半径と同じ)
 * 中心間距離=18:
 *   通常半径判定: 5+5=10 < 18 → 通常半径同士の判定ではヒットしない
 *   cancel_radius判定: 15+5=20 > 18 → ヒット(打ち消し発生)
 */
TEST(BulletCancelTest, ExpandedCancelRadius_CancelsOutsideNormalHitbox){
    CollisionSystem cs;
    BulletSystem player_bs(10);
    BulletSystem enemy_bs(10);

    // radius=5, cancel_radius=15 で自機弾を生成
    ASSERT_TRUE(player_bs.Spawn({0.0f, 0.0f}, {0.0f, -720.0f}, 5.0f, 15.0f));
    // 中心間距離=18 (通常半径 5+5=10 では届かない位置)
    ASSERT_TRUE(enemy_bs.Spawn( {18.0f, 0.0f}, {0.0f, 200.0f}, 5.0f));

    const auto result = cs.ResolvePlayerBulletsVsEnemyBullets(player_bs, enemy_bs);

    EXPECT_EQ(result.cancel_count, 1u);     // cancel_radiusで打ち消し成立
    EXPECT_EQ(player_bs.CountActive(), 0u);
    EXPECT_EQ(enemy_bs.CountActive(),  0u);
}

/**
 * @brief cancel_radiusの外側では打ち消しが発生しないことを確認
 *
 * 自機弾: radius=5, cancel_radius=15
 * 敵弾:   radius=5 (cancel_radiusは通常半径と同じ)
 * 中心間距離=25:
 *   cancel_radius判定: 15+5=20 < 25 → ヒットしない
 */
TEST(BulletCancelTest, OutsideCancelRadius_DoesNotCancel){
    CollisionSystem cs;
    BulletSystem player_bs(10);
    BulletSystem enemy_bs(10);

    // radius=5, cancel_radius=15 で自機弾を生成
    ASSERT_TRUE(player_bs.Spawn({0.0f, 0.0f}, {0.0f, -720.0f}, 5.0f, 15.0f));
    // 中心間距離=25 (cancel_radius 15+5=20 でも届かない位置)
    ASSERT_TRUE(enemy_bs.Spawn( {25.0f, 0.0f}, {0.0f, 200.0f}, 5.0f));

    const auto result = cs.ResolvePlayerBulletsVsEnemyBullets(player_bs, enemy_bs);

    EXPECT_EQ(result.cancel_count, 0u);     // cancel_radiusでも届かない
    EXPECT_EQ(player_bs.CountActive(), 1u);
    EXPECT_EQ(enemy_bs.CountActive(),  1u);
}
