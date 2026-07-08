#include <gtest/gtest.h>

#include "zimovka/core/Vec2.hpp"

using zimovka::Vec2;

// ──────────────────────────────────────────────────────
// コンストラクタ
// ──────────────────────────────────────────────────────
TEST(Vec2Test, DefaultConstruct) {
    Vec2 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST(Vec2Test, ValueConstruct) {
    Vec2 v{3.0f, -4.0f};
    EXPECT_FLOAT_EQ(v.x,  3.0f);
    EXPECT_FLOAT_EQ(v.y, -4.0f);
}

// ──────────────────────────────────────────────────────
// 算術演算子
// ──────────────────────────────────────────────────────
TEST(Vec2Test, Add) {
    Vec2 a{1.0f, 2.0f};
    Vec2 b{3.0f, 4.0f};
    Vec2 c = a + b;
    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 6.0f);
}

TEST(Vec2Test, Subtract) {
    Vec2 a{5.0f, 7.0f};
    Vec2 b{2.0f, 3.0f};
    Vec2 c = a - b;
    EXPECT_FLOAT_EQ(c.x, 3.0f);
    EXPECT_FLOAT_EQ(c.y, 4.0f);
}

TEST(Vec2Test, ScalarMultiply) {
    Vec2 v{2.0f, 3.0f};
    Vec2 r = v * 2.0f;
    EXPECT_FLOAT_EQ(r.x, 4.0f);
    EXPECT_FLOAT_EQ(r.y, 6.0f);
}

TEST(Vec2Test, ScalarMultiplyLeft) {
    Vec2 v{2.0f, 3.0f};
    Vec2 r = 2.0f * v;
    EXPECT_FLOAT_EQ(r.x, 4.0f);
    EXPECT_FLOAT_EQ(r.y, 6.0f);
}

TEST(Vec2Test, ScalarDivide) {
    Vec2 v{6.0f, 4.0f};
    Vec2 r = v / 2.0f;
    EXPECT_FLOAT_EQ(r.x, 3.0f);
    EXPECT_FLOAT_EQ(r.y, 2.0f);
}

TEST(Vec2Test, UnaryNegate) {
    Vec2 v{1.0f, -2.0f};
    Vec2 n = -v;
    EXPECT_FLOAT_EQ(n.x, -1.0f);
    EXPECT_FLOAT_EQ(n.y,  2.0f);
}

// ──────────────────────────────────────────────────────
// 複合代入演算子
// ──────────────────────────────────────────────────────
TEST(Vec2Test, AddAssign) {
    Vec2 v{1.0f, 2.0f};
    v += Vec2{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 6.0f);
}

TEST(Vec2Test, SubtractAssign) {
    Vec2 v{5.0f, 7.0f};
    v -= Vec2{2.0f, 3.0f};
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
}

// ──────────────────────────────────────────────────────
// メンバ関数
// ──────────────────────────────────────────────────────
TEST(Vec2Test, Length) {
    Vec2 v{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v.Length(), 5.0f);
}

TEST(Vec2Test, LengthSquare) {
    Vec2 v{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v.LengthSquare(), 25.0f);
}

TEST(Vec2Test, Dot_Perpendicular) {
    Vec2 a{1.0f, 0.0f};
    Vec2 b{0.0f, 1.0f};
    EXPECT_FLOAT_EQ(a.Dot(b), 0.0f);
}

TEST(Vec2Test, Dot_Parallel) {
    Vec2 a{1.0f, 0.0f};
    Vec2 b{2.0f, 0.0f};
    EXPECT_FLOAT_EQ(a.Dot(b), 2.0f);
}

TEST(Vec2Test, DistanceFrom) {
    Vec2 a{0.0f, 0.0f};
    Vec2 b{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(a.DistanceFrom(b), 5.0f);
}

TEST(Vec2Test, Normalized) {
    Vec2 v{3.0f, 4.0f};
    Vec2 n = v.Normalized();
    EXPECT_NEAR(n.Length(), 1.0f, 1e-6f);
    EXPECT_NEAR(n.x, 0.6f, 1e-6f);
    EXPECT_NEAR(n.y, 0.8f, 1e-6f);
}

TEST(Vec2Test, Normalized_ZeroVector) {
    // ゼロ除算ガード: ゼロベクトルを正規化してもゼロが返る
    Vec2 v{0.0f, 0.0f};
    Vec2 n = v.Normalized();
    EXPECT_FLOAT_EQ(n.x, 0.0f);
    EXPECT_FLOAT_EQ(n.y, 0.0f);
}

TEST(Vec2Test, IsZero_True) {
    EXPECT_TRUE(Vec2{}.IsZero());
}

TEST(Vec2Test, IsZero_False) {
    EXPECT_FALSE((Vec2{0.0f, 1.0f}.IsZero()));
    EXPECT_FALSE((Vec2{1.0f, 0.0f}.IsZero()));
}
