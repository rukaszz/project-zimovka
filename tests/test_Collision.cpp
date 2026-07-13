#include <gtest/gtest.h>

#include "zimovka/core/Circle.hpp"
#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/collision/CollisionSystem.hpp"
#include "zimovka/systems/collision/CollisionUtilities.hpp"
#include "zimovka/systems/player/Player.hpp"

using zimovka::BulletSystem;
using zimovka::Circle;
using zimovka::CollisionSystem;
using zimovka::Player;
using zimovka::Vec2;

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
    EXPECT_FALSE(cs.CheckPlayerHitByBullets(player, bs));
    // 全て非活性なので弾の走査用インデックスも0
    EXPECT_EQ(cs.LastCheckCount(), 0u);
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
 * @brief last_check_count_のactiveな弾カウントの確認
 * 
 */
TEST(CollisionSystemTest, LastCheckCount_TracksActive){
    CollisionSystem cs;
    BulletSystem    bs(5);
    Player player;
    player.position   = {0.0f, 0.0f};
    player.hit_radius = 1.0f;
    // 遠くに3発(どれも当たらない)
    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({901.0f, 901.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({902.0f, 902.0f}, {0.0f, 0.0f}, 1.0f);
    cs.CheckPlayerHitByBullets(player, bs);
    // 活性の弾が3つチェックされたので3
    EXPECT_EQ(cs.LastCheckCount(), 3u);
}

/**
 * @brief 非活性な弾の走査でlast_check_count_が増えないか確認
 *
 */
TEST(CollisionSystemTest, LastCheckCount_ResetEachCall){
    CollisionSystem cs;
    BulletSystem    bs(5);
    Player player;
    player.position   = {0.0f, 0.0f};
    player.hit_radius = 1.0f;
    // 活性状態の弾2つ(当たらない)
    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({901.0f, 901.0f}, {0.0f, 0.0f}, 1.0f);
    cs.CheckPlayerHitByBullets(player, bs);
    EXPECT_EQ(cs.LastCheckCount(), 2u);
    // 弾を消去(非活性化)
    bs.Clear();
    cs.CheckPlayerHitByBullets(player, bs);
    EXPECT_EQ(cs.LastCheckCount(), 0u); // Clear()で非活性化したのでゼロになる
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

    EXPECT_TRUE(cs.CheckPlayerHitByBullets(player, bs));
    // スロット 0 でヒット → early return → count == 1
    EXPECT_EQ(cs.LastCheckCount(), 1u);
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

    EXPECT_TRUE(cs.CheckPlayerHitByBullets(player, bs));
    // スロット0(miss→+1)+スロット1(hit→+1) → count=2, スロット2はスキップされう
    EXPECT_EQ(cs.LastCheckCount(), 2u);
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
    EXPECT_FALSE(cs.CheckPlayerHitByBullets(player, bs));
    EXPECT_EQ(cs.LastCheckCount(), 3u); // 3発を全件走査
}

/**
 * @brief inactiveな弾がlast_check_count_にカウントされないことを確認
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
    // Bulletの走査
    cs.CheckPlayerHitByBullets(player, bs);
    // inactiveな弾はskipされるのでカウントは0
    EXPECT_EQ(cs.LastCheckCount(), 0u);
}
