#include <gtest/gtest.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/systems/bomb/PlayerBombConfig.hpp"
#include "zimovka/systems/bomb/PlayerBombEvents.hpp"
#include "zimovka/systems/bomb/PlayerBombState.hpp"
#include "zimovka/systems/bomb/PlayerBombSystem.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/enemy/EnemySpawnParams.hpp"
#include "zimovka/systems/enemy/EnemySystem.hpp"

using zimovka::Action;
using zimovka::BulletSystem;
using zimovka::EnemySpawnParams;
using zimovka::EnemySystem;
using zimovka::InputState;
using zimovka::PlayerBombEvents;
using zimovka::PlayerBombSystem;
namespace PC = zimovka::PlayerBombConfig; // namespaceはエイリアスをつけて参照

// ヘルパ関数群
namespace{
/**
 * @brief ボムボタン押下のInputState作成
 * 
 * @return InputState 
 */
InputState BombPressed(){
    InputState s;
    s.SetPressed(Action::Bomb);
    return s;
}

/**
 * @brief 空のInputState生成
 * 
 * @return InputState 
 */
InputState NoInput(){
    return InputState{};
}

/**
 * @brief invincible_ticks_remainingが0になるまで空Tickを消費する
 * 
 * @param sys 
 * @param eb 
 * @param es 
 */
void DrainInvincible(PlayerBombSystem& sys, BulletSystem& eb, EnemySystem& es){
    for(std::uint32_t i = 0; i < PC::INVINCIBLE_TICKS; ++i){
        sys.UpdateTick(NoInput(), false, eb, es);
    }
}

/**
 * @brief stockを使い切るためのヘルパ
 * 
 * 無敵(invincible)期間もボム入力は入れる
 * 
 * @param sys 
 * @param eb 
 * @param es 
 */
void ExhaustStock(PlayerBombSystem& sys, BulletSystem& eb, EnemySystem& es){
    const std::uint32_t initial = sys.GetState().stock;
    for(std::uint32_t i = 0; i < initial; ++i){
        sys.UpdateTick(BombPressed(), false, eb, es);
        DrainInvincible(sys, eb, es);
    }
}

}   // anonymous namespace

// ──────────────────────────────────────────────────────
// 初期状態
// ──────────────────────────────────────────────────────
TEST(PlayerBombSystemTest, InitialState){
    PlayerBombSystem sys;
    const auto& s = sys.GetState();
    EXPECT_EQ(s.stock, 3u);
    EXPECT_FALSE(s.IsInvincible());
    EXPECT_FALSE(s.HasPendingHit());
    EXPECT_EQ(s.grace_ticks_remaining, 0u);
}

TEST(PlayerBombSystemTest, Reset_RestoresInitialState){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    sys.UpdateTick(BombPressed(), false, eb, es);   // stock--, 無敵開始
    sys.Reset();
    const auto& s = sys.GetState();
    EXPECT_EQ(s.stock, 3u);
    EXPECT_FALSE(s.IsInvincible());
    EXPECT_FALSE(s.HasPendingHit());
}

// ──────────────────────────────────────────────────────
// 通常ボム発動(状態C: 通常状態でボム入力)
// ──────────────────────────────────────────────────────
/**
 * @brief 通常のボム入力
 * 
 */
TEST(PlayerBombSystemTest, NormalBomb_Activated){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    const auto ev = sys.UpdateTick(BombPressed(), false, eb, es);
    // ボム発動イベントのみTRUE
    EXPECT_TRUE(ev.activated);
    EXPECT_FALSE(ev.hit_cancelled);
    EXPECT_FALSE(ev.hit_applied);
}

/**
 * @brief ボム入力によるボム残数消費チェック
 * 
 */
TEST(PlayerBombSystemTest, NormalBomb_ConsumesStock){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    sys.UpdateTick(BombPressed(), false, eb, es);
    // デフォルト3 → 2
    EXPECT_EQ(sys.GetState().stock, 2u);
}

/**
 * @brief ボムのあと無敵状態になったことを確認
 * 
 */
TEST(PlayerBombSystemTest, NormalBomb_StartsInvincibility){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    sys.UpdateTick(BombPressed(), false, eb, es);
    EXPECT_TRUE(sys.GetState().IsInvincible());
    EXPECT_EQ(sys.GetState().invincible_ticks_remaining, PC::INVINCIBLE_TICKS);
}

/**
 * @brief ボム実行後にEnemyBulletsが消えることを確認
 * 
 */
TEST(PlayerBombSystemTest, NormalBomb_ClearsEnemyBullets){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    // 敵弾生成
    (void)eb.Spawn({100.0f, 100.0f}, {0.0f, 10.0f}, 3.0f);
    (void)eb.Spawn({200.0f, 200.0f}, {0.0f, 10.0f}, 3.0f);
    // 2つ生成
    ASSERT_EQ(eb.CountActive(), 2u);

    // 敵弾が消え，消えた数を確認
    const auto ev = sys.UpdateTick(BombPressed(), false, eb, es);
    EXPECT_EQ(ev.cleared_bullet_count, 2u);
    EXPECT_EQ(eb.CountActive(), 0u);
}
/**
 * @brief activeな敵へダメージを与えることを確認
 * 
 */
TEST(PlayerBombSystemTest, NormalBomb_KillsActiveEnemies){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    // 最低限のデフォルトパラメータで敵を生成
    EnemySpawnParams p;
    p.position             = {240.0f, 200.0f};
    p.render_size          = {32.0f, 32.0f};
    p.hurtbox_radius       = 13.0f;
    p.contact_radius       = 10.0f;
    p.hp                   = 1;
    p.fire_interval_ticks  = 120;
    // 敵弾生成
    (void)es.Spawn(p);
    ASSERT_EQ(es.CountActive(), 1u);

    const auto ev = sys.UpdateTick(BombPressed(), false, eb, es);
    EXPECT_EQ(ev.enemy_kill_count, 1u);
    EXPECT_EQ(es.CountActive(), 0u);
}
/**
 * @brief ボム残数がゼロのとき，ボムの処理が実行されない
 * 
 */
TEST(PlayerBombSystemTest, NoStock_BombIgnored){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);

    ExhaustStock(sys, eb, es);  // ボムを消費
    EXPECT_EQ(sys.GetState().stock, 0u);

    const auto ev = sys.UpdateTick(BombPressed(), false, eb, es);
    EXPECT_FALSE(ev.activated);
    EXPECT_EQ(sys.GetState().stock, 0u);
}

// ──────────────────────────────────────────────────────
// 無敵状態(状態A)
// ──────────────────────────────────────────────────────
/**
 * @brief 無敵処理の確認
 * 
 * 無敵中は毎Tickでinvincible_ticks_remainingが減少する
 */
TEST(PlayerBombSystemTest, Invincible_CountsDown){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    sys.UpdateTick(BombPressed(), false, eb, es);
    const std::uint32_t before = sys.GetState().invincible_ticks_remaining;
    sys.UpdateTick(NoInput(), false, eb, es);
    EXPECT_EQ(sys.GetState().invincible_ticks_remaining, before - 1);
}

/**
 * @brief 無敵中の被弾が無視されるか
 * 
 */
TEST(PlayerBombSystemTest, Invincible_IgnoresHit){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    // ボム
    sys.UpdateTick(BombPressed(), false, eb, es);
    // ボム中に被弾
    const auto ev = sys.UpdateTick(NoInput(), true, eb, es);    // hit=true
    EXPECT_FALSE(ev.hit_applied);   // ボムは無効状態
    EXPECT_FALSE(ev.hit_cancelled);
    EXPECT_FALSE(sys.GetState().HasPendingHit());
}

/**
 * @brief 無敵中のボム入力は無視されることを確認
 * 
 */
TEST(PlayerBombSystemTest, Invincible_IgnoresBombInput){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    sys.UpdateTick(BombPressed(), false, eb, es);   // stock=2, 無敵開始
    const std::uint32_t stock_before = sys.GetState().stock;
    // 再度ボム入力
    const auto ev = sys.UpdateTick(BombPressed(), false, eb, es);
    EXPECT_FALSE(ev.activated);
    EXPECT_EQ(sys.GetState().stock, stock_before);  // stock 変化なし
}

/**
 * @brief INVINCIBLE_TICKS経過後に無敵が解除されるか確認
 * 
 */
TEST(PlayerBombSystemTest, Invincible_ExpiresAfterTicks){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    sys.UpdateTick(BombPressed(), false, eb, es);
    EXPECT_TRUE(sys.GetState().IsInvincible());
    
    DrainInvincible(sys, eb, es);   // 無敵時間消費
    EXPECT_FALSE(sys.GetState().IsInvincible());
}

// ──────────────────────────────────────────────────────
// 食らいボム受付中(状態B)
// ──────────────────────────────────────────────────────
/**
 * @brief 被弾Tickから食らいボム入力受け付けが開始するか
 * 
 */
TEST(PlayerBombSystemTest, Hit_StartsGracePeriod){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);

    const auto ev = sys.UpdateTick(NoInput(), true, eb, es);
    EXPECT_FALSE(ev.hit_applied);   // 被弾はまだ確定していない
    EXPECT_TRUE(sys.GetState().HasPendingHit());
    EXPECT_EQ(sys.GetState().grace_ticks_remaining, PC::GRACE_TICKS);
}

/**
 * @brief 被弾と同Tickにボム入力 → 被弾は即時キャンセル(状態Cのbranch)
 * 
 */
TEST(PlayerBombSystemTest, HitAndBomb_SameTick_CancelsHit){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);

    const auto ev = sys.UpdateTick(BombPressed(), true, eb, es);
    EXPECT_TRUE(ev.activated);      // ボム有効化
    EXPECT_TRUE(ev.hit_cancelled);  // 被弾キャンセルTRUE
    EXPECT_FALSE(ev.hit_applied);
    EXPECT_FALSE(sys.GetState().HasPendingHit());
}

/**
 * @brief 食らいボム入力受け付け中にボム入力あり → 被弾キャンセル(hit_cancelled)
 * 
 */
TEST(PlayerBombSystemTest, GracePeriod_BombInput_CancelsHit){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);

    sys.UpdateTick(NoInput(), true, eb, es);        // grace開始
    EXPECT_TRUE(sys.GetState().HasPendingHit());    // 受付中

    const auto ev = sys.UpdateTick(BombPressed(), false, eb, es);
    EXPECT_TRUE(ev.activated);      // ボム有効化
    EXPECT_TRUE(ev.hit_cancelled);  // 被弾キャンセル
    EXPECT_FALSE(ev.hit_applied);
    EXPECT_FALSE(sys.GetState().HasPendingHit());   // 食らいボム入力受け付け無効
}

/**
 * @brief 食らいボム入力受け付け時間超過で被弾確定
 * 
 * GRACE_TICKS経過で確定(hit_applied)
 */
TEST(PlayerBombSystemTest, GracePeriod_Timeout_AppliesHit){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    // 入力なし
    sys.UpdateTick(NoInput(), true, eb, es);        // grace開始(state C)
    EXPECT_EQ(sys.GetState().grace_ticks_remaining, PC::GRACE_TICKS);

    // GRACE_TICKS中のTickはhit_applied=falseである
    // GRACE_TICKS - 1まで確定はしない
    for(std::uint32_t i = 0; i < PC::GRACE_TICKS - 1; ++i){
        const auto ev = sys.UpdateTick(NoInput(), false, eb, es);
        EXPECT_FALSE(ev.hit_applied) << "Tick " << i;
    }

    // 最後のTickでhit_applied=trueで確定
    const auto ev = sys.UpdateTick(NoInput(), false, eb, es);
    EXPECT_TRUE(ev.hit_applied);
    EXPECT_FALSE(sys.GetState().HasPendingHit());
}

/**
 * @brief stock=0でも食らいボム受け付けは開始し, タイムアウトでhit_appliedになる
 * 
 */
TEST(PlayerBombSystemTest, GracePeriod_NoStock_StillAppliesHitOnTimeout){
    PlayerBombSystem sys;
    BulletSystem eb(100);
    EnemySystem  es(10);
    // 全ボム消費
    ExhaustStock(sys, eb, es);
    EXPECT_EQ(sys.GetState().stock, 0u);

    // 被弾
    sys.UpdateTick(NoInput(), true, eb, es);        // grace開始
    EXPECT_TRUE(sys.GetState().HasPendingHit());
    // 食らいボム猶予中
    for(std::uint32_t i = 0; i < PC::GRACE_TICKS - 1; ++i){
        sys.UpdateTick(NoInput(), false, eb, es);
    }
    // GRACE_TICKS最終で被弾確定
    const auto ev = sys.UpdateTick(NoInput(), false, eb, es);
    EXPECT_TRUE(ev.hit_applied);
}
