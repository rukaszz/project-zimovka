#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/boss/BossConfig.hpp"
#include "zimovka/systems/boss/BossDamageResult.hpp"
#include "zimovka/systems/boss/BossEvents.hpp"
#include "zimovka/systems/boss/BossStatus.hpp"
#include "zimovka/systems/boss/BossSystem.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/pattern/PatternSystem.hpp"

using zimovka::BossConfig;
using zimovka::BossDamageResult;
using zimovka::BossEvents;
using zimovka::BossStatus;
using zimovka::BossSystem;
using zimovka::BulletSystem;
using zimovka::PatternSystem;
using zimovka::Vec2;

// 有効なBossConfig(全テストで利用するので静的に定義)
static BossConfig MakeDefaultConfig(){
    BossConfig cfg;
    cfg.name                 = "TestBoss";
    cfg.position             = {320.0f, 80.0f};
    cfg.velocity             = {0.0f, 0.0f};
    cfg.render_size          = {64.0f, 64.0f};
    cfg.hurtbox_radius       = 30.0f;
    cfg.contact_radius       = 20.0f;
    cfg.max_hp               = 10;
    cfg.fight_tick           = 3;    // テスト用に短く設定
    cfg.aimed_fire_interval  = 5;
    cfg.circle_fire_interval = 10;
    return cfg;
}

// ──────────────────────────────────────────────────────
// 初期状態
// ──────────────────────────────────────────────────────
/**
 * @brief BossSystemの初期状態はInactive
 *
 */
TEST(BossSystemTest, InitialStateIsInactive){
    BossSystem bs;
    EXPECT_FALSE(bs.IsNotInactive());
    EXPECT_FALSE(bs.IsFighting());
    EXPECT_FALSE(bs.IsDefeated());
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Inactive);
}

// ──────────────────────────────────────────────────────
// Start()バリデーション
// ──────────────────────────────────────────────────────
/**
 * @brief 有効なConfigでStart()が例外を投げないことを確認
 *
 */
TEST(BossSystemTest, StartSucceedsWithValidConfig){
    BossSystem bs;
    EXPECT_NO_THROW(bs.Start(MakeDefaultConfig()));
}

/**
 * @brief nameが空のConfigでStart()が例外を投げることを確認
 *
 */
TEST(BossSystemTest, StartRejectsEmptyName){
    BossSystem bs;
    BossConfig cfg = MakeDefaultConfig();
    cfg.name = "";
    EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
}

/**
 * @brief Vec2フィールドが無限大のConfigでStart()が例外を投げることを確認
 *
 */
TEST(BossSystemTest, StartRejectsInfiniteVec2){
    const float inf = std::numeric_limits<float>::infinity();
    {   // position
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.position = {inf, 0.0f};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // velocity
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.velocity = {0.0f, inf};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // render_size
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.render_size = {inf, 64.0f};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
}

/**
 * @brief max_hpなどの数値フィールドが不正なConfigでStart()が例外を投げることを確認
 *
 */
TEST(BossSystemTest, StartRejectsInvalidNumericFields){
    {   // max_hp=0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.max_hp = 0;
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // hurtbox_radius=0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.hurtbox_radius = 0.0f;
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // contact_radius=0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.contact_radius = 0.0f;
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // fight_tick=0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.fight_tick = 0;
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // aimed_fire_interval=0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.aimed_fire_interval = 0;
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // circle_fire_interval=0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.circle_fire_interval = 0;
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
}

// ──────────────────────────────────────────────────────
// Start()通過後(boss_.state != BossStatus::Inactive)
// ──────────────────────────────────────────────────────
/**
 * @brief Start()後はEntering状態になることを確認
 *
 */
TEST(BossSystemTest, StartTransitionsToEntering){
    BossSystem bs;
    bs.Start(MakeDefaultConfig());
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Entering);
    EXPECT_TRUE(bs.IsNotInactive());
    EXPECT_FALSE(bs.IsFighting());
}

/**
 * @brief Start()後はConfigの値がBossにコピーされることを確認
 *
 */
TEST(BossSystemTest, StartCopiesConfig){
    BossSystem bs;
    BossConfig cfg = MakeDefaultConfig();
    bs.Start(cfg);
    // configの内容が反映されているか確認
    const auto& boss = bs.GetBoss();
    EXPECT_EQ(boss.name, "TestBoss");
    EXPECT_FLOAT_EQ(boss.position.x, 320.0f);
    EXPECT_FLOAT_EQ(boss.position.y, 80.0f);
    EXPECT_EQ(boss.hp, 10);
    EXPECT_EQ(boss.max_hp, 10);
    EXPECT_FLOAT_EQ(boss.hurtbox_radius, 30.0f);
    EXPECT_FLOAT_EQ(boss.contact_radius, 20.0f);
    EXPECT_EQ(boss.aimed_fire_interval,  5u);
    EXPECT_EQ(boss.circle_fire_interval, 10u);
}

// ──────────────────────────────────────────────────────
// Reset()
// ──────────────────────────────────────────────────────
/**
 * @brief Reset()でHPが最大値に戻ることを確認
 *
 */
TEST(BossSystemTest, ResetRestoresHp){
    BossSystem bs;
    bs.Start(MakeDefaultConfig());

    bs.TakeDamage(5);
    EXPECT_EQ(bs.GetBoss().hp, 5);

    bs.Reset();
    EXPECT_EQ(bs.GetBoss().hp, 10);
}

// ──────────────────────────────────────────────────────
// UpdateTick()状態遷移
// ──────────────────────────────────────────────────────
/**
 * @brief Inactive時はNoneを返すことを確認
 *
 */
TEST(BossSystemTest, UpdateTickReturnsNoneWhenInactive){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_EQ(
        bs.UpdateTick(1.0f/60.0f, player_pos, patterns, enemy_bullets),
        BossEvents::None
    );
}

/**
 * @brief Entering中はWaitingTriggerを返すことを確認
 *
 */
TEST(BossSystemTest, UpdateTickReturnsWaitingTriggerDuringEntering){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.fight_tick = 60;    // 十分な長さに設定
    bs.Start(cfg);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Entering);

    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_EQ(
        bs.UpdateTick(1.0f/60.0f, player_pos, patterns, enemy_bullets),
        BossEvents::WaitingTrigger
    );
}

/**
 * @brief fight_ticks消費でEntering→Fightingへ遷移することを確認
 *
 */
TEST(BossSystemTest, UpdateTickTransitionsEnteringToFighting){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.fight_tick = 3;
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // 3tick消費でFightingへ
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets);
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets);
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets);

    EXPECT_EQ(bs.GetBoss().status, BossStatus::Fighting);
    EXPECT_TRUE(bs.IsFighting());
}

/**
 * @brief Fighting中はActiveを返すことを確認
 *
 */
TEST(BossSystemTest, UpdateTickReturnsActiveWhenFighting){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.fight_tick = 1;
    cfg.aimed_fire_interval  = 999;  // 発射させない
    cfg.circle_fire_interval = 999;
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // Entering→Fighting
    EXPECT_EQ(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets),
        BossEvents::Active
    );
}

/**
 * @brief Defeating状態はDefeatedを返すことを確認
 *
 */
TEST(BossSystemTest, UpdateTickReturnsDefeatedWhenDefeating){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.fight_tick = 1;
    cfg.max_hp = 1;
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // Entering→Fighting
    bs.TakeDamage(1);                                       // HP→0, Defeating遷移
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    EXPECT_TRUE(bs.IsDefeated());

    EXPECT_EQ(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets),
        BossEvents::Defeated
    );
}

// ──────────────────────────────────────────────────────
// UpdateTick()弾発射
// ──────────────────────────────────────────────────────
/**
 * @brief タイマーが0になると弾がenem_bulletsプールに追加されることを確認(EmitSpread)
 *
 */
TEST(BossSystemTest, FiringEmitsSpreadBullets){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.fight_tick           = 1;
    cfg.aimed_fire_interval  = 2;   // 2tick後に発射
    cfg.circle_fire_interval = 999; // 全方向弾は発射させない
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // fight_tick=1: Entering→Fighting
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // aimed_fire_timer: 2→1
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // aimed_fire_timer: 1→0 → 発射
    EXPECT_GT(enemy_bullets.CountActive(), 0u);
}

/**
 * @brief タイマーが0になると弾がenemy_bulletsプールに追加されることを確認(EmitCircle)
 *
 */
TEST(BossSystemTest, FiringEmitsCircleBullets){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.fight_tick           = 1;
    cfg.aimed_fire_interval  = 999; // 自機狙い弾は発射させない
    cfg.circle_fire_interval = 2;   // 2tick後に発射
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // fight_tick=1: Entering→Fighting
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // circle_fire_timer: 2→1
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // circle_fire_timer: 1→0 → 発射
    EXPECT_GT(enemy_bullets.CountActive(), 0u);
}

// ──────────────────────────────────────────────────────
// TakeDamage()
// ──────────────────────────────────────────────────────
/**
 * @brief Inactive時はInvalidTargetを返すことを確認
 *
 */
TEST(BossSystemTest, TakeDamageRejectsWhenInactive){
    BossSystem bs;
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.GetBoss().hp, 0);
}

/**
 * @brief damage<=0はInvalidTargetを返すことを確認
 *
 */
TEST(BossSystemTest, TakeDamageRejectsNonPositiveDamage){
    BossSystem bs;
    bs.Start(MakeDefaultConfig());
    EXPECT_EQ(bs.TakeDamage(0),  BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.TakeDamage(-1), BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.GetBoss().hp, 10); // HPは変化しない
}

/**
 * @brief 通常ダメージでDamagedを返しHPが減ることを確認
 *
 */
TEST(BossSystemTest, TakeDamageReducesHp){
    BossSystem bs;
    bs.Start(MakeDefaultConfig());
    EXPECT_EQ(bs.TakeDamage(3), BossDamageResult::Damaged);
    EXPECT_EQ(bs.GetBoss().hp, 7);
}

/**
 * @brief HPが0以下になるとDestroyedを返しDefeating遷移することを確認
 *
 */
TEST(BossSystemTest, TakeDamageTransitionsToDefeating){
    BossSystem bs;
    bs.Start(MakeDefaultConfig());
    EXPECT_EQ(bs.TakeDamage(10), BossDamageResult::Destroyed);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    EXPECT_TRUE(bs.IsDefeated());
}

/**
 * @brief 超過ダメージ(HPを超える量)でもDestroyedを返すことを確認
 *
 */
TEST(BossSystemTest, TakeDamageOverkillReturnsDestroyed){
    BossSystem bs;
    bs.Start(MakeDefaultConfig());
    EXPECT_EQ(bs.TakeDamage(999), BossDamageResult::Destroyed);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
}

// ──────────────────────────────────────────────────────
// 当たり判定円
// ──────────────────────────────────────────────────────
/**
 * @brief GetHurtboxCircle / GetContactCircle がpositionとradiusを正しく返すことを確認
 *
 */
TEST(BossSystemTest, GetCirclesReturnCorrectValues){
    BossSystem bs;
    bs.Start(MakeDefaultConfig());
    const auto& boss = bs.GetBoss();
    const auto hurtbox = boss.GetHurtboxCircle();
    const auto contact = boss.GetContactCircle();
    EXPECT_FLOAT_EQ(hurtbox.center.x, 320.0f);
    EXPECT_FLOAT_EQ(hurtbox.center.y, 80.0f);
    EXPECT_FLOAT_EQ(hurtbox.radius,   30.0f);
    EXPECT_FLOAT_EQ(contact.center.x, 320.0f);
    EXPECT_FLOAT_EQ(contact.center.y, 80.0f);
    EXPECT_FLOAT_EQ(contact.radius,   20.0f);
}
