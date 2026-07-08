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
// CollisionUtilities::Intersects のテスト
// ──────────────────────────────────────────────────────
TEST(IntersectsTest, Overlapping) {
    Circle a{{0.0f, 0.0f}, 5.0f};
    Circle b{{3.0f, 0.0f}, 5.0f};
    EXPECT_TRUE(zimovka::CollisionUtilities::Intersects(a, b));
}

TEST(IntersectsTest, Touching) {
    // 中心距離 == r1 + r2 (接触 = 被弾)
    Circle a{{0.0f, 0.0f}, 5.0f};
    Circle b{{10.0f, 0.0f}, 5.0f};
    EXPECT_TRUE(zimovka::CollisionUtilities::Intersects(a, b));
}

TEST(IntersectsTest, Separated) {
    Circle a{{0.0f, 0.0f}, 3.0f};
    Circle b{{100.0f, 0.0f}, 3.0f};
    EXPECT_FALSE(zimovka::CollisionUtilities::Intersects(a, b));
}

TEST(IntersectsTest, SameCenter) {
    Circle a{{5.0f, 5.0f}, 1.0f};
    Circle b{{5.0f, 5.0f}, 1.0f};
    EXPECT_TRUE(zimovka::CollisionUtilities::Intersects(a, b));
}

TEST(IntersectsTest, JustOutside) {
    // 中心距離が r1 + r2 をわずかに超える
    Circle a{{0.0f, 0.0f}, 5.0f};
    Circle b{{10.001f, 0.0f}, 5.0f};
    EXPECT_FALSE(zimovka::CollisionUtilities::Intersects(a, b));
}

TEST(IntersectsTest, ZeroRadiusPoint) {
    // 半径0の点どうし: 同一座標なら接触
    Circle a{{3.0f, 3.0f}, 0.0f};
    Circle b{{3.0f, 3.0f}, 0.0f};
    EXPECT_TRUE(zimovka::CollisionUtilities::Intersects(a, b));
}

// ──────────────────────────────────────────────────────
// CollisionSystem のテスト
// ──────────────────────────────────────────────────────
TEST(CollisionSystemTest, NoHit_EmptyBullets) {
    CollisionSystem cs;
    BulletSystem   bs(10);
    Player player;
    player.position = {480.0f, 360.0f};
    EXPECT_FALSE(cs.CheckPlayerHitByBullets(player, bs));
    EXPECT_EQ(cs.LastCheckCount(), 0u);
}

TEST(CollisionSystemTest, Hit_BulletOnPlayer) {
    CollisionSystem cs;
    BulletSystem   bs(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 10.0f;
    // プレイヤーの中心に弾を置く
    bs.Spawn({100.0f, 100.0f}, {0.0f, 0.0f}, 5.0f);
    EXPECT_TRUE(cs.CheckPlayerHitByBullets(player, bs));
}

TEST(CollisionSystemTest, NoHit_BulletFarAway) {
    CollisionSystem cs;
    BulletSystem   bs(10);
    Player player;
    player.position   = {100.0f, 100.0f};
    player.hit_radius = 4.0f;
    bs.Spawn({900.0f, 600.0f}, {0.0f, 0.0f}, 3.0f);
    EXPECT_FALSE(cs.CheckPlayerHitByBullets(player, bs));
}

TEST(CollisionSystemTest, LastCheckCount_TracksActive) {
    CollisionSystem cs;
    BulletSystem   bs(5);
    Player player;
    player.position   = {0.0f, 0.0f};
    player.hit_radius = 1.0f;
    // 遠くに3発(どれも当たらない)
    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({901.0f, 901.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({902.0f, 902.0f}, {0.0f, 0.0f}, 1.0f);
    cs.CheckPlayerHitByBullets(player, bs);
    EXPECT_EQ(cs.LastCheckCount(), 3u);
}

TEST(CollisionSystemTest, LastCheckCount_ResetEachCall) {
    CollisionSystem cs;
    BulletSystem   bs(5);
    Player player;
    player.position   = {0.0f, 0.0f};
    player.hit_radius = 1.0f;

    bs.Spawn({900.0f, 900.0f}, {0.0f, 0.0f}, 1.0f);
    bs.Spawn({901.0f, 901.0f}, {0.0f, 0.0f}, 1.0f);
    cs.CheckPlayerHitByBullets(player, bs);
    EXPECT_EQ(cs.LastCheckCount(), 2u);

    bs.Clear();
    cs.CheckPlayerHitByBullets(player, bs);
    EXPECT_EQ(cs.LastCheckCount(), 0u); // Clear后はゼロになる
}
