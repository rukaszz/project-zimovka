#include <gtest/gtest.h>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"

using zimovka::BulletSystem;
using zimovka::Vec2;

// ──────────────────────────────────────────────────────
// 初期状態
// ──────────────────────────────────────────────────────
/**
 * @brief BulletSystemの生成をテスト
 * 
 */
TEST(BulletSystemTest, InitialState){
    // 最大弾数10でインスタンス化
    BulletSystem bs(10);
    // 弾は全て非活性
    EXPECT_EQ(bs.CountActive(), 0u);
    // 弾数は10個
    EXPECT_EQ(bs.GetCapacity(), 10u);
}

// ──────────────────────────────────────────────────────
// Spawn
// ──────────────────────────────────────────────────────
/**
 * @brief Spawn()の正常な呼び出し
 * 
 */
TEST(BulletSystemTest, SpawnSucceeds){
    BulletSystem bs(10);
    // 1つ生成
    EXPECT_TRUE(bs.Spawn(Vec2{100.0f, 100.0f}, Vec2{0.0f, 60.0f}, 3.0f));
    EXPECT_EQ(bs.CountActive(), 1u);
}

/**
 * @brief 半径0.0fの弾生成
 * 
 */
TEST(BulletSystemTest, SpawnInvalidRadius_Zero){
    BulletSystem bs(10);
    // 半径0
    EXPECT_FALSE(bs.Spawn(Vec2{0.0f, 0.0f}, Vec2{0.0f, 1.0f}, 0.0f));
    // 無効な半径なので生成されていない
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief 負の半径の弾生成
 * 
 */
TEST(BulletSystemTest, SpawnInvalidRadius_Negative){
    BulletSystem bs(10);
    // 半径-1.0f
    EXPECT_FALSE(bs.Spawn(Vec2{0.0f, 0.0f}, Vec2{0.0f, 1.0f}, -1.0f));
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief 弾の生成限界までSpawn
 * 
 */
TEST(BulletSystemTest, SpawnFillsPool){
    // max_bullets=3で生成
    BulletSystem bs(3);
    EXPECT_TRUE(bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f));
    EXPECT_TRUE(bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f));
    EXPECT_TRUE(bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f));
    EXPECT_EQ(bs.CountActive(), 3u);
}

/**
 * @brief 弾の生成限界を超えてSpawn
 * 
 */
TEST(BulletSystemTest, SpawnFailsWhenPoolFull){
    BulletSystem bs(2);
    bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);   // プール満杯
    // もう生成できない
    EXPECT_FALSE(bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f));
}

// ──────────────────────────────────────────────────────
// Clear
// ──────────────────────────────────────────────────────
/**
 * @brief 弾を全て非活性にするClear()呼び出し
 * 
 */
TEST(BulletSystemTest, ClearResetsActiveCount){
    BulletSystem bs(10);
    // 弾が2つ活性状態
    bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    // 全て非活性になる
    bs.Clear();
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief Clear後にまたSpawnできることを確認
 * 
 */
TEST(BulletSystemTest, ClearAllowsRespawn){
    BulletSystem bs(2);
    bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    bs.Clear();
    EXPECT_TRUE(bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f));
}

// ──────────────────────────────────────────────────────
// Update: 移動と画面外消去
// ──────────────────────────────────────────────────────
/**
 * @brief 通常のUpdate()処理
 * 
 */
TEST(BulletSystemTest, UpdateKeepsBulletOnScreen){
    BulletSystem bs(5);
    // 画面中心から微速で移動する弾(1フレームでは出ない)
    bs.Spawn(Vec2{480.0f, 360.0f}, Vec2{1.0f, 0.0f}, 3.0f);
    bs.Update((1.0f/60.0f), 960.0f, 720.0f);
    // 1回のUpdate()では画面内に収まり活性状態のまあm
    EXPECT_EQ(bs.CountActive(), 1u);
}

/**
 * @brief Update()にて画面外へ出た弾が非活性になることを確認
 * 
 */
TEST(BulletSystemTest, UpdateMovesOutOfScreen){
    BulletSystem bs(5);
    // 画面外(遠く離れた場所)へ高速で飛ぶ弾を生成
    bs.Spawn(Vec2{480.0f, 360.0f}, Vec2{-99999.0f, -99999.0f}, 1.0f);
    // スクリーンサイズは(960.0f, 720.0f)
    bs.Update((1.0f/60.0f), 960.0f, 720.0f);
    // 弾は画面外へ吹き飛び非活性になる
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief Update()処理時の座標計算の確認
 *
 */
TEST(BulletSystemTest, GetBullets_PositionUpdated){
    BulletSystem bs(5);
    // x軸方向へ60px/s動く弾を生成
    bs.Spawn(Vec2{0.0f, 0.0f}, Vec2{60.0f, 0.0f}, 1.0f);
    bs.Update((1.0f/60.0f), 960.0f, 720.0f); // 1フレーム進める
    // 1/60秒 * 60px/s = 1px動く
    const auto bullets = bs.GetBullets();
    // 浮動小数点誤差を加味してEXPECT_NEAR()を用いる(許容誤差0．0001)
    EXPECT_NEAR(bullets[0].position.x, 1.0f, 1e-4f);
}

// ──────────────────────────────────────────────────────
// メモリアドレスの連続性・プールの再確保なし
// ──────────────────────────────────────────────────────
/**
 * @brief GetBullets()が返すspanの要素がメモリ上で連続していることを確認
 *
 * vector<Bullet>によるオブジェクトプールは連続メモリが保証される
 */
TEST(BulletSystemTest, GetBullets_ContiguousMemory){
    BulletSystem bs(5);
    const auto bullets = bs.GetBullets();
    // 次のテストに進むために3要素分のメモリ確保は必要
    ASSERT_GE(bullets.size(), 3u);
    // &演算子でアドレスを取得
    // &bullets[i+1] == &bullets[i] + 1 が成り立つ = 連続メモリ
    EXPECT_EQ(&bullets[1], &bullets[0] + 1);
    EXPECT_EQ(&bullets[2], &bullets[0] + 1 + 1);
}

/**
 * @brief Spawn()を繰り返してもGetBullets().data()が変化しないことを確認
 *
 * コンストラクタでresize()を実行済み → Spawn()はスロットの状態遷移のみで再確保なし
 */
TEST(BulletSystemTest, GetBullets_DataPointer_StableAfterSpawn){
    BulletSystem bs(10);
    // 生成直後のBulletSystemのポインタ(先頭)を取得
    const zimovka::Bullet* ptr_before = bs.GetBullets().data();
    for(int i = 0; i < 5; ++i){
        bs.Spawn(Vec2{static_cast<float>(i), 0.0f}, Vec2{0.0f, 1.0f}, 1.0f);
    }
    // ポインタが変化していない = ヒープ再確保なし
    EXPECT_EQ(bs.GetBullets().data(), ptr_before);
}

/**
 * @brief Clear()後もGetBullets().data()が変化しないことを確認
 *
 * Clear()はactiveフラグをfalseにするだけでvectorを変更しない
 */
TEST(BulletSystemTest, GetBullets_DataPointer_StableAfterClear){
    BulletSystem bs(10);
    // 生成直後のポインタ取得
    const zimovka::Bullet* ptr_before = bs.GetBullets().data();
    bs.Spawn(Vec2{0,0}, Vec2{0,1}, 1.0f);
    bs.Clear();
    EXPECT_EQ(bs.GetBullets().data(), ptr_before);
}

// ──────────────────────────────────────────────────────
// 画面外消去の方向別境界テスト
// ──────────────────────────────────────────────────────
/**
 * @brief 左方向へ高速移動した弾が非活性になることを確認
 * 
 */
TEST(BulletSystemTest, OutOfScreen_Left){
    BulletSystem bs(5);
    bs.Spawn(Vec2{1.0f, 360.0f}, Vec2{-9999.0f, 0.0f}, 3.0f);
    bs.Update(1.0f, 960.0f, 720.0f);
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief 右方向へ高速移動した弾が非活性になることを確認
 * 
 */
TEST(BulletSystemTest, OutOfScreen_Right){
    BulletSystem bs(5);
    bs.Spawn(Vec2{959.0f, 360.0f}, Vec2{9999.0f, 0.0f}, 3.0f);
    bs.Update(1.0f, 960.0f, 720.0f);
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief 上方向へ高速移動した弾が非活性になることを確認
 * 
 */
TEST(BulletSystemTest, OutOfScreen_Top){
    BulletSystem bs(5);
    bs.Spawn(Vec2{480.0f, 1.0f}, Vec2{0.0f, -9999.0f}, 3.0f);
    bs.Update(1.0f, 960.0f, 720.0f);
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief 下方向へ高速移動した弾が非活性になることを確認
 * 
 */
TEST(BulletSystemTest, OutOfScreen_Bottom){
    BulletSystem bs(5);
    bs.Spawn(Vec2{480.0f, 719.0f}, Vec2{0.0f, 9999.0f}, 3.0f);
    bs.Update(1.0f, 960.0f, 720.0f);
    EXPECT_EQ(bs.CountActive(), 0u);
}

/**
 * @brief 静止した弾が画面内で活性を維持することを確認
 * 
 */
TEST(BulletSystemTest, NotOutOfScreen_Stationary){
    BulletSystem bs(5);
    bs.Spawn(Vec2{480.0f, 360.0f}, Vec2{0.0f, 0.0f}, 3.0f); // 静止
    bs.Update(1.0f, 960.0f, 720.0f);
    EXPECT_EQ(bs.CountActive(), 1u);
}
