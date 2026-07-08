#include <gtest/gtest.h>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"

using zimovka::BulletSystem;
using zimovka::Vec2;

// ──────────────────────────────────────────────────────
// 初期状態
// ──────────────────────────────────────────────────────
TEST(BulletSystemTest, InitialState) {
    BulletSystem sys(10);
    EXPECT_EQ(sys.CountActive(), 0u);
    EXPECT_EQ(sys.GetCapacity(), 10u);
}

// ──────────────────────────────────────────────────────
// Spawn
// ──────────────────────────────────────────────────────
TEST(BulletSystemTest, SpawnSucceeds) {
    BulletSystem sys(10);
    EXPECT_TRUE(sys.Spawn(Vec2{100.0f, 100.0f}, Vec2{0.0f, 60.0f}, 3.0f));
    EXPECT_EQ(sys.CountActive(), 1u);
}

TEST(BulletSystemTest, SpawnInvalidRadius_Zero) {
    BulletSystem sys(10);
    EXPECT_FALSE(sys.Spawn(Vec2{0.0f, 0.0f}, Vec2{0.0f, 1.0f}, 0.0f));
    EXPECT_EQ(sys.CountActive(), 0u);
}

TEST(BulletSystemTest, SpawnInvalidRadius_Negative) {
    BulletSystem sys(10);
    EXPECT_FALSE(sys.Spawn(Vec2{0.0f, 0.0f}, Vec2{0.0f, 1.0f}, -1.0f));
    EXPECT_EQ(sys.CountActive(), 0u);
}

TEST(BulletSystemTest, SpawnFillsPool) {
    BulletSystem sys(3);
    EXPECT_TRUE(sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f));
    EXPECT_TRUE(sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f));
    EXPECT_TRUE(sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f));
    EXPECT_EQ(sys.CountActive(), 3u);
}

TEST(BulletSystemTest, SpawnFailsWhenPoolFull) {
    BulletSystem sys(2);
    sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    EXPECT_FALSE(sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f)); // プール満杯
}

// ──────────────────────────────────────────────────────
// Clear
// ──────────────────────────────────────────────────────
TEST(BulletSystemTest, ClearResetsActiveCount) {
    BulletSystem sys(10);
    sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    sys.Clear();
    EXPECT_EQ(sys.CountActive(), 0u);
}

TEST(BulletSystemTest, ClearAllowsRespawn) {
    // Clear後にまたSpawnできることを確認
    BulletSystem sys(2);
    sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    sys.Clear();
    EXPECT_TRUE(sys.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f));
}

// ──────────────────────────────────────────────────────
// Update: 移動と画面外消去
// ──────────────────────────────────────────────────────
TEST(BulletSystemTest, UpdateMovesOutOfScreen) {
    BulletSystem sys(5);
    // 画面外(遠く離れた場所)へ高速で飛ぶ弾を生成
    sys.Spawn(Vec2{480.0f, 360.0f}, Vec2{-99999.0f, -99999.0f}, 1.0f);
    sys.Update(1.0f, 960.0f, 720.0f);
    EXPECT_EQ(sys.CountActive(), 0u);
}

TEST(BulletSystemTest, UpdateKeepsBulletOnScreen) {
    BulletSystem sys(5);
    // 画面中心から微速で移動する弾(1フレームでは出ない)
    sys.Spawn(Vec2{480.0f, 360.0f}, Vec2{1.0f, 0.0f}, 3.0f);
    sys.Update(1.0f / 60.0f, 960.0f, 720.0f);
    EXPECT_EQ(sys.CountActive(), 1u);
}

TEST(BulletSystemTest, GetBullets_PositionUpdated) {
    BulletSystem sys(5);
    sys.Spawn(Vec2{0.0f, 0.0f}, Vec2{60.0f, 0.0f}, 1.0f);
    sys.Update(1.0f / 60.0f, 960.0f, 720.0f); // 1フレーム進める
    // 1/60秒 × 60px/s = 1px 動く
    const auto bullets = sys.GetBullets();
    EXPECT_NEAR(bullets[0].position.x, 1.0f, 1e-4f);
}
