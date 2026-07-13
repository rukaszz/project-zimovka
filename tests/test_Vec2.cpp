#include <gtest/gtest.h>

#include "zimovka/core/Vec2.hpp"

using zimovka::Vec2;

// ──────────────────────────────────────────────────────
// コンストラクタ
// ──────────────────────────────────────────────────────
/**
 * @brief デフォルトコンストラクタでのVec2生成
 * 
 */
TEST(Vec2Test, DefaultConstruct){
    Vec2 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

/**
 * @brief コンストラクタに引数を渡してVec2生成
 * 
 */
TEST(Vec2Test, ValueConstruct){
    Vec2 v{3.0f, -4.0f};
    EXPECT_FLOAT_EQ(v.x,  3.0f);
    EXPECT_FLOAT_EQ(v.y, -4.0f);
}

// ──────────────────────────────────────────────────────
// 算術演算子
// ──────────────────────────────────────────────────────
/**
 * @brief 加算演算子オーバーロードのテスト
 * 
 */
TEST(Vec2Test, Add){
    Vec2 a{1.0f, 2.0f};
    Vec2 b{3.0f, 4.0f};
    Vec2 c = a + b;
    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 6.0f);
}

/**
 * @brief 減算演算子オーバーロードのテスト
 * 
 */
TEST(Vec2Test, Subtract){
    Vec2 a{5.0f, 7.0f};
    Vec2 b{2.0f, 3.0f};
    Vec2 c = a - b;
    EXPECT_FLOAT_EQ(c.x, 3.0f);
    EXPECT_FLOAT_EQ(c.y, 4.0f);
}

/**
 * @brief 積算演算子オーバーロードの右項スカラでのテスト
 * 
 */
TEST(Vec2Test, ScalarMultiply){
    Vec2 v{2.0f, 3.0f};
    Vec2 r = v * 2.0f;
    EXPECT_FLOAT_EQ(r.x, 4.0f);
    EXPECT_FLOAT_EQ(r.y, 6.0f);
}

/**
 * @brief 積算演算子オーバーロードの左項スカラでのテスト
 * 
 */
TEST(Vec2Test, ScalarMultiplyLeft){
    Vec2 v{2.0f, 3.0f};
    Vec2 r = 2.0f * v;
    EXPECT_FLOAT_EQ(r.x, 4.0f);
    EXPECT_FLOAT_EQ(r.y, 6.0f);
}

/**
 * @brief 除算演算子オーバーロードの右項スカラでのテスト
 * 
 */
TEST(Vec2Test, ScalarDivide){
    Vec2 v{6.0f, 4.0f};
    Vec2 r = v / 2.0f;
    EXPECT_FLOAT_EQ(r.x, 3.0f);
    EXPECT_FLOAT_EQ(r.y, 2.0f);
}

/**
 * @brief 単項減算演算子による符号反転のテスト
 * 
 */
TEST(Vec2Test, UnaryNegate){
    Vec2 v{1.0f, -2.0f};
    Vec2 n = -v;
    EXPECT_FLOAT_EQ(n.x, -1.0f);
    EXPECT_FLOAT_EQ(n.y,  2.0f);
}

// ──────────────────────────────────────────────────────
// 複合代入演算子
// ──────────────────────────────────────────────────────
/**
 * @brief 複合加算代入演算子のテスト
 * 
 */
TEST(Vec2Test, AddAssign){
    Vec2 v{1.0f, 2.0f};
    v += Vec2{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 6.0f);
}

/**
 * @brief 複合減算代入演算子のテスト
 * 
 */
TEST(Vec2Test, SubtractAssign){
    Vec2 v{5.0f, 7.0f};
    v -= Vec2{2.0f, 3.0f};
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
}

// ──────────────────────────────────────────────────────
// メンバ関数
// ──────────────────────────────────────────────────────
/**
 * @brief 2次元ベクトルのノルム計算
 * 
 */
TEST(Vec2Test, Length){
    Vec2 v{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v.Length(), 5.0f);
}

/**
 * @brief 2次元ベクトルのノルムの2乗
 * 
 */
TEST(Vec2Test, LengthSquare){
    Vec2 v{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v.LengthSquare(), 25.0f);
}

/**
 * @brief 2次元ベクトルの内積計算(直行)
 * 
 */
TEST(Vec2Test, Dot_Perpendicular){
    Vec2 a{1.0f, 0.0f};
    Vec2 b{0.0f, 1.0f};
    EXPECT_FLOAT_EQ(a.Dot(b), 0.0f);
}

/**
 * @brief 2次元ベクトルの内積計算(並行)
 * 
 */
TEST(Vec2Test, Dot_Parallel){
    Vec2 a{1.0f, 0.0f};
    Vec2 b{2.0f, 0.0f};
    EXPECT_FLOAT_EQ(a.Dot(b), 2.0f);
}

/**
 * @brief 2次元ベクトル同士の距離
 * 
 */
TEST(Vec2Test, DistanceFrom){
    Vec2 a{0.0f, 0.0f};
    Vec2 b{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(a.DistanceFrom(b), 5.0f);
}

/**
 * @brief 2次元ベクトルの正規化
 * 
 */
TEST(Vec2Test, Normalized){
    Vec2 v{3.0f, 4.0f};
    Vec2 n = v.Normalized();
    // 浮動小数点の除算なので誤差を許容する(±0.000001)
    EXPECT_NEAR(n.Length(), 1.0f, 1e-6f);
    EXPECT_NEAR(n.x, 0.6f, 1e-6f);
    EXPECT_NEAR(n.y, 0.8f, 1e-6f);
}

/**
 * @brief 正規化処理にてゼロ除算を防止するガードのテスト
 * 
 */
TEST(Vec2Test, Normalized_ZeroVector){
    // ゼロ除算ガード: ゼロベクトルを正規化してもゼロが返る
    Vec2 v{0.0f, 0.0f};
    Vec2 n = v.Normalized();
    EXPECT_FLOAT_EQ(n.x, 0.0f);
    EXPECT_FLOAT_EQ(n.y, 0.0f);
}

/**
 * @brief 要素が全てゼロのベクトル判定関数テスト
 * 
 */
TEST(Vec2Test, IsZero_True){
    EXPECT_TRUE(Vec2{}.IsZero());
}

/**
 * @brief 要素が非ゼロなベクトルへのゼロベクトル判定関数
 * 
 */
TEST(Vec2Test, IsZero_False){
    EXPECT_FALSE((Vec2{0.0f, 1.0f}.IsZero()));
    EXPECT_FALSE((Vec2{1.0f, 0.0f}.IsZero()));
}
