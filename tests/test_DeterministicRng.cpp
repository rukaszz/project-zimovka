#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

#include "zimovka/core/DeterministicRng.hpp"

using zimovka::DeterministicRng;

// ──────────────────────────────────────────────────────
// 既知のメルセンヌツイスタの乱数値
// ──────────────────────────────────────────────────────
/**
 * @brief seed=0のときの最初の出力値が既知の値と一致することを確認
 *
 * std::mt19937(0) の最初の出力は規格上一意に定まる
 * 複数の環境や将来のリグレッションで値が変わっていないかを検出する
 */
TEST(DeterministicRngTest, KnownValue_Seed0_FirstOutput){
    DeterministicRng rng(0u);
    // std::mt19937 with seed 0: first value = 2357136044
    EXPECT_EQ(rng.NextU32(), 2357136044u);
}

/**
 * @brief 独立したstd::mt19937(0)と連続100回の出力が一致することを確認
 *
 * DeterministicRngがmt19937を正確にラップしており，
 * 余分な消費・スキップが起きていないことを検証する
 */
TEST(DeterministicRngTest, KnownValues_MatchesMt19937_Seed0){
    std::mt19937    ref(0u);
    DeterministicRng rng(0u);
    for(int i = 0; i < 100; ++i){
        EXPECT_EQ(
            rng.NextU32(),
            static_cast<std::uint32_t>(ref())
        ) << "i=" << i;
    }
}

/**
 * @brief 別のシード値でも独立したstd::mt19937と出力が一致することを確認
 */
TEST(DeterministicRngTest, KnownValues_MatchesMt19937_Seed12345){
    std::mt19937     ref(12345u);
    DeterministicRng rng(12345u);
    for(int i = 0; i < 100; ++i){
        EXPECT_EQ(
            rng.NextU32(),
            static_cast<std::uint32_t>(ref())
        ) << "i=" << i;
    }
}

// ──────────────────────────────────────────────────────
// 同一シードで同一の値が得られるか
// ──────────────────────────────────────────────────────
/**
 * @brief 同一シードで構築した2つのインスタンスが同一の列を返すことを確認
 *
 * リプレイ機能の基盤となる性質
 */
TEST(DeterministicRngTest, SameSeed_SameSequence){
    DeterministicRng rng1(42u);
    DeterministicRng rng2(42u);
    for(int i = 0; i < 200; ++i){
        EXPECT_EQ(rng1.NextU32(), rng2.NextU32()) << "i=" << i;
    }
}

/**
 * @brief 異なるシードでは最初の出力値が異なることを確認
 */
TEST(DeterministicRngTest, DifferentSeeds_DifferentFirstValue){
    DeterministicRng rng0(0u);
    DeterministicRng rng1(1u);
    EXPECT_NE(rng0.NextU32(), rng1.NextU32());
}

// ──────────────────────────────────────────────────────
// Reseed
// ──────────────────────────────────────────────────────
/**
 * @brief Reseed()後に同一シードで呼び出すと同一の列が復元されることを確認
 *
 * 乱数消費後にReseedすれば再び同一列が得られることでリプレイが可能になる
 */
TEST(DeterministicRngTest, Reseed_RestoresSequence){
    DeterministicRng rng(99u);
    // 最初のN個を取得
    std::vector<std::uint32_t> first;
    for(int i = 0; i < 50; ++i){
        first.push_back(rng.NextU32());
    }
    // 同じシードでReseed
    rng.Reseed(99u);
    for(int i = 0; i < 50; ++i){
        EXPECT_EQ(rng.NextU32(), first[static_cast<std::size_t>(i)]) << "i=" << i;
    }
}

/**
 * @brief 異なるシードでReseedした後は異なる列になることを確認
 */
TEST(DeterministicRngTest, Reseed_NewSeed_DifferentValue){
    DeterministicRng rng(0u);
    const std::uint32_t before = rng.NextU32();
    rng.Reseed(1u);
    const std::uint32_t after = rng.NextU32();
    EXPECT_NE(before, after);
}

// ──────────────────────────────────────────────────────
// UniformU32
// ──────────────────────────────────────────────────────
/**
 * @brief UniformU32()の戻り値が[min, max]に収まることを確認
 */
TEST(DeterministicRngTest, UniformU32_InRange){
    DeterministicRng rng(0u);
    constexpr std::uint32_t MIN = 10u;
    constexpr std::uint32_t MAX = 100u;
    for(int i = 0; i < 1000; ++i){
        const std::uint32_t v = rng.UniformU32(MIN, MAX);
        EXPECT_GE(v, MIN) << "i=" << i;
        EXPECT_LE(v, MAX) << "i=" << i;
    }
}

/**
 * @brief min==maxのとき常にminが返ることを確認
 *
 * range=1なので棄却サンプリングが起きずoffset=0となりminを返す
 */
TEST(DeterministicRngTest, UniformU32_MinEqualsMax_ReturnsMin){
    DeterministicRng rng(0u);
    for(int i = 0; i < 20; ++i){
        EXPECT_EQ(rng.UniformU32(7u, 7u), 7u) << "i=" << i;
    }
}

/**
 * @brief min=0, max=UINT32_MAXの全域指定で例外が出ず値が返ることを確認
 *
 * range=2^32のとき棄却閾値limit=2^32なので常に通過する
 */
TEST(DeterministicRngTest, UniformU32_FullRange_NoException){
    DeterministicRng rng(0u);
    constexpr std::uint32_t UMAX = std::numeric_limits<std::uint32_t>::max();
    for(int i = 0; i < 10; ++i){
        EXPECT_NO_THROW(rng.UniformU32(0u, UMAX)) << "i=" << i;
    }
}

/**
 * @brief min > maxのときstd::invalid_argumentが投げられることを確認
 */
TEST(DeterministicRngTest, UniformU32_MinGtMax_Throws){
    DeterministicRng rng(0u);
    EXPECT_THROW(rng.UniformU32(10u, 5u), std::invalid_argument);
}

/**
 * @brief UniformU32()も同一シードで同一の列を返すことを確認
 */
TEST(DeterministicRngTest, UniformU32_SameSeed_SameSequence){
    DeterministicRng rng1(7u);
    DeterministicRng rng2(7u);
    for(int i = 0; i < 100; ++i){
        EXPECT_EQ(
            rng1.UniformU32(0u, 999u),
            rng2.UniformU32(0u, 999u)
        ) << "i=" << i;
    }
}

// ──────────────────────────────────────────────────────
// UnitFloat: [0, 1) の境界値
// ──────────────────────────────────────────────────────
/**
 * @brief UnitFloat()が[0, 1)の範囲に収まることを確認
 *
 * 実装は(NextU32() >> 8) / 2^24 なので，
 * 最小値=0.0f, 最大値=(2^24-1)/2^24 < 1.0f であることを保証できる
 */
TEST(DeterministicRngTest, UnitFloat_InRange){
    DeterministicRng rng(0u);
    for(int i = 0; i < 1000; ++i){
        const float v = rng.UnitFloat();
        EXPECT_GE(v, 0.0f) << "i=" << i;
        EXPECT_LT(v, 1.0f) << "i=" << i;
    }
}

/**
 * @brief UnitFloat()が負の値を返さないことを確認(別シードでも同様)
 */
TEST(DeterministicRngTest, UnitFloat_NeverNegative){
    DeterministicRng rng(42u);
    for(int i = 0; i < 1000; ++i){
        EXPECT_GE(rng.UnitFloat(), 0.0f) << "i=" << i;
    }
}

/**
 * @brief UnitFloat()が1.0f以上の値を返さないことを確認
 *
 * 最大値は (2^24 - 1) / 2^24 ≈ 0.99999994f であり，1.0fを超えない
 */
TEST(DeterministicRngTest, UnitFloat_NeverReachesOne){
    // 理論的最大値: (2^24 - 1) / 2^24
    constexpr float THEORETICAL_MAX = 16'777'215.0f / 16'777'216.0f;
    DeterministicRng rng(0u);
    for(int i = 0; i < 1000; ++i){
        const float v = rng.UnitFloat();
        EXPECT_LE(v, THEORETICAL_MAX) << "i=" << i;
    }
}

/**
 * @brief UnitFloat()が有限値のみを返すことを確認(NaN/Infにならない)
 */
TEST(DeterministicRngTest, UnitFloat_AlwaysFinite){
    DeterministicRng rng(0u);
    for(int i = 0; i < 1000; ++i){
        EXPECT_TRUE(std::isfinite(rng.UnitFloat())) << "i=" << i;
    }
}

/**
 * @brief UnitFloat()も同一シードで同一の列を返すことを確認
 */
TEST(DeterministicRngTest, UnitFloat_SameSeed_SameSequence){
    DeterministicRng rng1(3u);
    DeterministicRng rng2(3u);
    for(int i = 0; i < 200; ++i){
        EXPECT_EQ(rng1.UnitFloat(), rng2.UnitFloat()) << "i=" << i;
    }
}

// ──────────────────────────────────────────────────────
// UniformFloat
// ──────────────────────────────────────────────────────
/**
 * @brief UniformFloat()の戻り値が[min, max)に収まることを確認
 *
 * UnitFloat()が[0, 1)なので，min + (max-min)*UnitFloat() は
 * [min, max)の半開区間に収まる
 */
TEST(DeterministicRngTest, UniformFloat_InRange){
    DeterministicRng rng(0u);
    constexpr float MIN = -5.0f;
    constexpr float MAX =  5.0f;
    for(int i = 0; i < 1000; ++i){
        const float v = rng.UniformFloat(MIN, MAX);
        EXPECT_GE(v, MIN) << "i=" << i;
        EXPECT_LT(v, MAX) << "i=" << i;
    }
}

/**
 * @brief 正の範囲でもUniformFloat()が収まることを確認
 */
TEST(DeterministicRngTest, UniformFloat_PositiveRange){
    DeterministicRng rng(0u);
    constexpr float MIN = 10.0f;
    constexpr float MAX = 20.0f;
    for(int i = 0; i < 500; ++i){
        const float v = rng.UniformFloat(MIN, MAX);
        EXPECT_GE(v, MIN) << "i=" << i;
        EXPECT_LT(v, MAX) << "i=" << i;
    }
}

/**
 * @brief min==maxのとき常にminが返ることを確認
 *
 * (max - min) * UnitFloat() = 0 * UnitFloat() = 0 なのでmin+0=minが返る
 */
TEST(DeterministicRngTest, UniformFloat_MinEqualsMax_ReturnsMin){
    DeterministicRng rng(0u);
    for(int i = 0; i < 20; ++i){
        EXPECT_FLOAT_EQ(rng.UniformFloat(3.0f, 3.0f), 3.0f) << "i=" << i;
    }
}

/**
 * @brief min > maxのときstd::invalid_argumentが投げられることを確認
 */
TEST(DeterministicRngTest, UniformFloat_MinGtMax_Throws){
    DeterministicRng rng(0u);
    EXPECT_THROW(rng.UniformFloat(10.0f, 5.0f), std::invalid_argument);
}

/**
 * @brief minがNaNのときstd::invalid_argumentが投げられることを確認
 */
TEST(DeterministicRngTest, UniformFloat_NanMin_Throws){
    DeterministicRng rng(0u);
    EXPECT_THROW(
        rng.UniformFloat(std::numeric_limits<float>::quiet_NaN(), 1.0f),
        std::invalid_argument
    );
}

/**
 * @brief maxがNaNのときstd::invalid_argumentが投げられることを確認
 */
TEST(DeterministicRngTest, UniformFloat_NanMax_Throws){
    DeterministicRng rng(0u);
    EXPECT_THROW(
        rng.UniformFloat(0.0f, std::numeric_limits<float>::quiet_NaN()),
        std::invalid_argument
    );
}

/**
 * @brief minが+Infのときstd::invalid_argumentが投げられることを確認
 */
TEST(DeterministicRngTest, UniformFloat_InfMin_Throws){
    DeterministicRng rng(0u);
    EXPECT_THROW(
        rng.UniformFloat(std::numeric_limits<float>::infinity(), 1.0f),
        std::invalid_argument
    );
}

/**
 * @brief maxが+Infのときstd::invalid_argumentが投げられることを確認
 */
TEST(DeterministicRngTest, UniformFloat_InfMax_Throws){
    DeterministicRng rng(0u);
    EXPECT_THROW(
        rng.UniformFloat(0.0f, std::numeric_limits<float>::infinity()),
        std::invalid_argument
    );
}

/**
 * @brief UniformFloat()も同一シードで同一の列を返すことを確認
 */
TEST(DeterministicRngTest, UniformFloat_SameSeed_SameSequence){
    DeterministicRng rng1(5u);
    DeterministicRng rng2(5u);
    for(int i = 0; i < 200; ++i){
        EXPECT_EQ(
            rng1.UniformFloat(0.0f, 100.0f),
            rng2.UniformFloat(0.0f, 100.0f)
        ) << "i=" << i;
    }
}
