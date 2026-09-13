#include <gtest/gtest.h>

#include <cmath>
#include <limits>

#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/pattern/PatternEmitRequest.hpp"
#include "zimovka/systems/pattern/PatternSystem.hpp"

using zimovka::BulletSystem;
using zimovka::PatternEmitRequest;
using zimovka::PatternSystem;

namespace{
/**
 * @brief 引数で渡すためのPatternEmitRequest作成ヘルパ
 * 
 * bullet_countのみ指定可能としている
 * 
 * @param count 
 * @return PatternEmitRequest 
 */
PatternEmitRequest MakeRequest(std::uint32_t count = 5){
    PatternEmitRequest req{};
    req.origin         = {240.0f, 100.0f};
    req.base_angle_rad = 0.0f;
    req.bullet_count   = count;
    req.spread_rad     = 1.0f;
    req.bullet_speed   = 180.0f;
    req.bullet_radius  = 3.0f;
    return req;
}
}   // anonymous namespace

// ──────────────────────────────────────────────────────
// 正常系
// ──────────────────────────────────────────────────────
/**
 * @brief リクエストした弾の数が生成されるか
 * 
 */
TEST(PatternSystemTest, SpawnedCount_MatchesRequest){
    PatternSystem ps;
    BulletSystem  bs(100);
    EXPECT_EQ(ps.EmitSpread(MakeRequest(5), bs), 5u);
    EXPECT_EQ(bs.CountActive(), 5u);
}

/**
 * @brief 1way弾を発射する
 * 
 * spread_rad=0のとき，base_angle_rad=0のとき，右方向に発射されるか
 * ※begin_angle = base - spread/2 なのでspread=0のときbegin_angle == base_angleになる
 */
TEST(PatternSystemTest, OneBullet_ZeroSpread_DirectionMatchesBaseAngle){
    PatternSystem ps;
    BulletSystem  bs(10);
    // 1発のみの1way弾を発射
    PatternEmitRequest req = MakeRequest(1);
    req.base_angle_rad = 0.0f;  // 右向き
    req.spread_rad     = 0.0f;  // 1way弾→begin_angle = base_angle
    req.bullet_speed   = 100.0f;
    // 発射数1回
    EXPECT_EQ(ps.EmitSpread(req, bs), 1u);
    // 右方向に1発発射されている
    const auto& b = bs.GetBullets()[0];
    EXPECT_NEAR(b.velocity.x, 100.0f, 1e-4f);
    EXPECT_NEAR(b.velocity.y,   0.0f, 1e-4f);
}

// 

/**
 * @brief 2way弾を発射し，対称的に弾が広がるかを調べる
 * 
 * bullet_count=2のとき, 2発の弾はbase_angle_radを中心に±spread_rad/2で対称に広がる
 */
TEST(PatternSystemTest, TwoBullets_SymmetricAroundBaseAngle){
    PatternSystem ps;
    BulletSystem  bs(10);

    PatternEmitRequest req = MakeRequest(2);
    req.base_angle_rad = 0.0f;
    req.spread_rad     = 1.0f;     // 角度範囲: -0.5f〜+0.5f rad
    req.bullet_speed   = 1.0f;
    EXPECT_EQ(ps.EmitSpread(req, bs), 2u);

    // begin_angle = 0 - 0.5 = -0.5, step = 1.0 / (2-1) = 1.0
    const auto& b0 = bs.GetBullets()[0];    // angle = -0.5f
    const auto& b1 = bs.GetBullets()[1];    // angle = +0.5f
    // 1発目左
    EXPECT_NEAR(b0.velocity.x, std::cos(-0.5f), 1e-4f);
    EXPECT_NEAR(b0.velocity.y, std::sin(-0.5f), 1e-4f);
    // 2発目右
    EXPECT_NEAR(b1.velocity.x, std::cos( 0.5f), 1e-4f);
    EXPECT_NEAR(b1.velocity.y, std::sin( 0.5f), 1e-4f);
}

/**
 * @brief spread_radの処理をチェック
 * 
 * spread_rad=0 のとき全弾が同一方向を向く
 */
TEST(PatternSystemTest, ZeroSpread_AllBulletsSameDirection){
    PatternSystem ps;
    BulletSystem  bs(10);

    PatternEmitRequest req = MakeRequest(3);
    req.base_angle_rad = 0.0f;  // 右向き
    req.spread_rad     = 0.0f;  // 拡散角度ゼロ=base_angle_rad
    req.bullet_speed   = 1.0f;
    EXPECT_EQ(ps.EmitSpread(req, bs), 3u);
    // 全弾右へ
    for(std::size_t i = 0; i < 3; ++i){
        EXPECT_NEAR(bs.GetBullets()[i].velocity.x, 1.0f, 1e-4f);
        EXPECT_NEAR(bs.GetBullets()[i].velocity.y, 0.0f, 1e-4f);
    }
}

// ──────────────────────────────────────────────────────
// 引数バリデーション
// ──────────────────────────────────────────────────────
/**
 * @brief 発射数(bullet_count)がゼロ発
 * 
 */
TEST(PatternSystemTest, ZeroBulletCount_ReturnsZero){
    PatternSystem ps;
    BulletSystem  bs(10);
    // 0発はreturn 0で返る
    EXPECT_EQ(ps.EmitSpread(MakeRequest(0), bs), 0u);
    // 発射されていない
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief 拡散角度(spread_rad)が負
 * 
 * Y軸増加方向が下なので，上方向への発射になるような引数
 */
TEST(PatternSystemTest, NegativeSpread_ReturnsZero){
    PatternSystem ps;
    BulletSystem  bs(10);
    auto req = MakeRequest(5);
    req.spread_rad = -0.1f;
    // return 0になる
    EXPECT_EQ(ps.EmitSpread(req, bs), 0u);
}

/**
 * @brief 弾速(bullet_speed)がゼロ
 * 
 */
TEST(PatternSystemTest, ZeroSpeed_ReturnsZero){
    PatternSystem ps;
    BulletSystem  bs(10);
    auto req = MakeRequest(5);
    req.bullet_speed = 0.0f;
    EXPECT_EQ(ps.EmitSpread(req, bs), 0u);
}

/**
 * @brief 弾速(bullet_speed)が負
 * 
 */
TEST(PatternSystemTest, NegativeSpeed_ReturnsZero){
    PatternSystem ps;
    BulletSystem  bs(10);
    auto req = MakeRequest(5);
    req.bullet_speed = -1.0f;
    EXPECT_EQ(ps.EmitSpread(req, bs), 0u);
}

/**
 * @brief 弾拡散角度(bullet_radius)がゼロ
 * 
 */
TEST(PatternSystemTest, ZeroRadius_ReturnsZero){
    PatternSystem ps;
    BulletSystem  bs(10);
    auto req = MakeRequest(5);
    req.bullet_radius = 0.0f;
    EXPECT_EQ(ps.EmitSpread(req, bs), 0u);
}

/**
 * @brief 発射元座標(origin)がNaN
 * 
 */
TEST(PatternSystemTest, NanOriginX_ReturnsZero){
    PatternSystem ps;
    BulletSystem  bs(10);
    auto req = MakeRequest(5);
    req.origin.x = std::numeric_limits<float>::quiet_NaN();
    EXPECT_EQ(ps.EmitSpread(req, bs), 0u);
}

/**
 * @brief 拡散の基となる傾き(base_angle_rad)がNaN
 * 
 */
TEST(PatternSystemTest, InfBaseAngle_ReturnsZero){
    PatternSystem ps;
    BulletSystem  bs(10);
    auto req = MakeRequest(5);
    req.base_angle_rad = std::numeric_limits<float>::infinity();
    EXPECT_EQ(ps.EmitSpread(req, bs), 0u);
}

// ──────────────────────────────────────────────────────
// プール容量
// ──────────────────────────────────────────────────────
/**
 * @brief プール容量より大きい弾数の発射を要請
 * 
 * available < bullet_count → 一切生成しない(部分生成もしない)
 */
TEST(PatternSystemTest, PoolInsufficient_ReturnsZero){
    PatternSystem ps;
    BulletSystem  bs(3);    // 容量3 < 要求5
    EXPECT_EQ(ps.EmitSpread(MakeRequest(5), bs), 0u);
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief プールの空き容量ちょうどで発射要請
 * 
 * available == bullet_count → 全弾生成できる
 */
TEST(PatternSystemTest, PoolExact_AllSpawned){
    PatternSystem ps;
    BulletSystem  bs(5);    // 容量ちょうど
    EXPECT_EQ(ps.EmitSpread(MakeRequest(5), bs), 5u);
    EXPECT_EQ(bs.CountActive(), 5u);
}
