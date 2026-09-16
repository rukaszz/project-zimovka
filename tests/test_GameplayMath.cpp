#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/math/GameplayMath.hpp"

using zimovka::Vec2;
namespace GM = zimovka::GameplayMath;

// ──────────────────────────────────────────────────────
// AimAngleRad: 原点(0,0)からの基本4方向
// ──────────────────────────────────────────────────────
/**
 * @brief targetが右 → atan2(0, 1) = 0
 * 
 */
TEST(GameplayMathTest, AimAngleRad_TargetRight){
    EXPECT_NEAR(GM::AimAngleRad({0.0f, 0.0f}, {1.0f, 0.0f}), 0.0f, 1e-5f);
}
/**
 * @brief targetが下 → atan2(+1, 0) = +π/2
 * 
 * ※SDL座標系: y軸増加方向は下
 */
TEST(GameplayMathTest, AimAngleRad_TargetDown){
    const float expected = std::numbers::pi_v<float> / 2.0f;
    EXPECT_NEAR(GM::AimAngleRad({0.0f, 0.0f}, {0.0f, 1.0f}), expected, 1e-5f);
}
/**
 * @brief targetが上 → atan2(-1, 0) = -π/2
 * 
 */
TEST(GameplayMathTest, AimAngleRad_TargetUp){
    const float expected = -std::numbers::pi_v<float> / 2.0f;
    EXPECT_NEAR(GM::AimAngleRad({0.0f, 0.0f}, {0.0f, -1.0f}), expected, 1e-5f);
}
/**
 * @brief targetが左 → atan2(0, -1) = ±π
 * 
 */
TEST(GameplayMathTest, AimAngleRad_TargetLeft){
    const float angle = GM::AimAngleRad({0.0f, 0.0f}, {-1.0f, 0.0f});
    EXPECT_NEAR(std::abs(angle), std::numbers::pi_v<float>, 1e-5f);
}
/**
 * @brief originが(0,0)ではないとき，deltaの計算が正しく行われるか
 * 
 */
TEST(GameplayMathTest, AimAngleRad_NonZeroOrigin){
    // delta = (1, 0) → angle = 0
    EXPECT_NEAR(GM::AimAngleRad({100.0f, 200.0f}, {101.0f, 200.0f}), 0.0f, 1e-5f);
}
/**
 * @brief origin == targetのときatan2(0, 0) = 0であることを確認
 * 
 * ZimovkaのGameplayMathは同一点を0 radと定義する
 */
TEST(GameplayMathTest, AimAngleRad_SamePoint_ReturnsZero){
    EXPECT_NEAR(GM::AimAngleRad({5.0f, 5.0f}, {5.0f, 5.0f}), 0.0f, 1e-5f);
}

// ──────────────────────────────────────────────────────
// VelocityFromAngle: 基本4方向
// ──────────────────────────────────────────────────────
/**
 * @brief 右方向への弾速計算
 * 
 * angle=0 → {speed, 0}
 */
TEST(GameplayMathTest, VelocityFromAngle_Rightward){
    const Vec2 v = GM::VelocityFromAngle(0.0f, 100.0f);
    EXPECT_NEAR(v.x, 100.0f, 1e-4f);
    EXPECT_NEAR(v.y,   0.0f, 1e-4f);
}
/**
 * @brief 左方向への弾速計算
 * 
 * angle=0 → {-speed, 0}
 */
TEST(GameplayMathTest, VelocityFromAngle_Leftward){
    const float pi = std::numbers::pi_v<float>;
    const Vec2 v = GM::VelocityFromAngle(pi, 100.0f);
    EXPECT_NEAR(v.x, -100.0f, 1e-4f);
    EXPECT_NEAR(v.y,    0.0f, 1e-4f);
}
/**
 * @brief 下方向への弾速計算
 * 
 * angle=+π/2 → {0, speed}
 */
TEST(GameplayMathTest, VelocityFromAngle_Downward){
    const float pi = std::numbers::pi_v<float>;
    const Vec2 v = GM::VelocityFromAngle(pi / 2.0f, 100.0f);
    EXPECT_NEAR(v.x,   0.0f, 1e-4f);
    EXPECT_NEAR(v.y, 100.0f, 1e-4f);
}
/**
 * @brief 上方向への弾速計算
 * 
 * angle=-π/2 → {0, -speed}
 */
TEST(GameplayMathTest, VelocityFromAngle_Upward){
    const float pi = std::numbers::pi_v<float>;
    const Vec2 v = GM::VelocityFromAngle(-pi / 2.0f, 100.0f);
    EXPECT_NEAR(v.x,    0.0f, 1e-4f);
    EXPECT_NEAR(v.y, -100.0f, 1e-4f);
}

// ──────────────────────────────────────────────────────
// VelocityFromAngle: 速度スカラーの契約
// ──────────────────────────────────────────────────────
/**
 * @brief speedのスケールが正しく反映されるか
 * 
 * ※2つのベクトルに対して，2倍の速度差をつけると，座標も2倍になるかということ
 */
TEST(GameplayMathTest, VelocityFromAngle_SpeedScaled){
    const Vec2 v1 = GM::VelocityFromAngle(0.5f, 100.0f);
    const Vec2 v2 = GM::VelocityFromAngle(0.5f, 200.0f);
    EXPECT_NEAR(v2.x, v1.x * 2.0f, 1e-4f);
    EXPECT_NEAR(v2.y, v1.y * 2.0f, 1e-4f);
}
/**
 * @brief 速度ベクトルの大きさがspeedと一致する
 * 
 */
TEST(GameplayMathTest, VelocityFromAngle_MagnitudeEqualsSpeed){
    const float speed = 180.0f;
    const Vec2 v = GM::VelocityFromAngle(1.23f, speed);
    const float mag = std::sqrt(v.x * v.x + v.y * v.y); // 2点間の距離
    EXPECT_NEAR(mag, speed, 1e-3f);
}

// ──────────────────────────────────────────────────────
// 合成: AimAngleRad → VelocityFromAngle
// ──────────────────────────────────────────────────────
/**
 * @brief 2点間の距離と速度のスケールが一致するか
 * 
 * origin → target方向の正規化ベクトルとVelocityFromAngle(speed=1)が一致する
 */
TEST(GameplayMathTest, AimAndVelocity_Composition_DirectionMatches){
    // 2点間の距離=1
    const Vec2 origin = {100.0f, 200.0f};
    const Vec2 target = {200.0f, 100.0f};
    // 速度1.0fで設定
    const float angle = GM::AimAngleRad(origin, target);
    const Vec2  vel   = GM::VelocityFromAngle(angle, 1.0f);
    // 2点の差
    const Vec2  delta = target - origin;
    const float len   = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    // 2点間の距離と速度1.0fは一致する
    EXPECT_NEAR(vel.x, delta.x / len, 1e-5f);
    EXPECT_NEAR(vel.y, delta.y / len, 1e-5f);
}
