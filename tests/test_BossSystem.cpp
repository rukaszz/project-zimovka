#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/engine/update/UpdatePipeline.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/stages/Stage1Data.hpp"
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
using zimovka::InputState;
using zimovka::PatternSystem;
using zimovka::UpdatePipeline;
using zimovka::Vec2;

// ──────────────────────────────────────────────────────
// ヘルパ
// ──────────────────────────────────────────────────────

// 有効なBossConfig(全テストで利用するので静的に定義)
static BossConfig MakeDefaultConfig(){
    BossConfig cfg;
    cfg.name                     = "TestBoss";
    cfg.position                 = {320.0f, 80.0f};
    cfg.velocity                 = {0.0f, 0.0f};
    cfg.render_size              = {64.0f, 64.0f};
    cfg.hurtbox_radius           = 30.0f;
    cfg.contact_radius           = 20.0f;
    cfg.max_hp                   = 10;
    cfg.entering_duration_ticks  = 3;    // テスト用に短く設定
    cfg.aimed_fire_interval      = 5;
    cfg.circle_fire_interval     = 10;
    return cfg;
}

// Entering → Fighting への進行ヘルパ
static void AdvanceToFighting(
    BossSystem& bs, BulletSystem& bullets, PatternSystem& patterns)
{
    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    while(bs.GetBoss().status == BossStatus::Entering){
        bs.UpdateTick(dt, player_pos, patterns, bullets);
    }
}

// ──────────────────────────────────────────────────────
// 初期状態
// ──────────────────────────────────────────────────────
/**
 * @brief BossSystemの初期状態はInactive
 */
TEST(BossSystemTest, InitialStateIsInactive){
    BossSystem bs;
    EXPECT_FALSE(bs.IsNotInactive());
    EXPECT_FALSE(bs.IsFighting());
    EXPECT_FALSE(bs.IsDefeated());
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Inactive);
}

// ──────────────────────────────────────────────────────
// Start() バリデーション
// ──────────────────────────────────────────────────────
/**
 * @brief 有効なConfigでStart()が例外を投げないことを確認
 */
TEST(BossSystemTest, StartSucceedsWithValidConfig){
    BossSystem bs;
    EXPECT_NO_THROW(bs.Start(MakeDefaultConfig()));
}

/**
 * @brief nameが空のConfigでStart()が例外を投げることを確認
 */
TEST(BossSystemTest, StartRejectsEmptyName){
    BossSystem bs;
    BossConfig cfg = MakeDefaultConfig();
    cfg.name = "";
    EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
}

/**
 * @brief Vec2フィールドが無限大のConfigでStart()が例外を投げることを確認
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
 * @brief 数値フィールドが不正なConfigでStart()が例外を投げることを確認
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
    {   // entering_duration_ticks=0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.entering_duration_ticks = 0;
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
// Start() 後の状態
// ──────────────────────────────────────────────────────
/**
 * @brief Start()後はEntering状態になることを確認
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
 */
TEST(BossSystemTest, StartCopiesConfig){
    BossSystem bs;
    BossConfig cfg = MakeDefaultConfig();
    bs.Start(cfg);
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
 * @brief 撃破後にReset()するとInactive・HP・タイマーがすべて初期化されることを確認
 */
TEST(BossSystemTest, Reset_RestoresInactiveState){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);

    // ダメージを与えて撃破する
    bs.TakeDamage(cfg.max_hp);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);

    // Reset後は完全初期化されていること
    bs.Reset();
    const auto& boss = bs.GetBoss();
    EXPECT_EQ(boss.status, BossStatus::Inactive);
    EXPECT_FALSE(bs.IsNotInactive());
    EXPECT_EQ(boss.hp, 0);               // デフォルト値(0)に戻る
    EXPECT_EQ(boss.aimed_fire_timer,  0u);
    EXPECT_EQ(boss.circle_fire_timer, 0u);
}

// ──────────────────────────────────────────────────────
// UpdateTick() 状態遷移
// ──────────────────────────────────────────────────────
/**
 * @brief Inactive時はNoneを返すことを確認
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
 */
TEST(BossSystemTest, UpdateTickReturnsWaitingTriggerDuringEntering){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 60;    // 十分な長さに設定
    bs.Start(cfg);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Entering);

    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_EQ(
        bs.UpdateTick(1.0f/60.0f, player_pos, patterns, enemy_bullets),
        BossEvents::WaitingTrigger
    );
}

/**
 * @brief entering_duration_ticks消費でEntering→Fightingへ遷移することを確認
 */
TEST(BossSystemTest, UpdateTickTransitionsEnteringToFighting){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 3;
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets);
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets);
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets);

    EXPECT_EQ(bs.GetBoss().status, BossStatus::Fighting);
    EXPECT_TRUE(bs.IsFighting());
}

/**
 * @brief Fighting中はActiveを返すことを確認
 */
TEST(BossSystemTest, UpdateTickReturnsActiveWhenFighting){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
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
 */
TEST(BossSystemTest, UpdateTickReturnsDefeatedWhenDefeating){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.max_hp = 1;
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);  // HP→0, Defeating遷移
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    EXPECT_TRUE(bs.IsDefeated());

    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_EQ(
        bs.UpdateTick(1.0f/60.0f, player_pos, patterns, enemy_bullets),
        BossEvents::Defeated
    );
}

// ──────────────────────────────────────────────────────
// UpdateTick() 弾発射
// ──────────────────────────────────────────────────────
/**
 * @brief タイマーが0になるとenem_bulletsプールに追加されることを確認(EmitSpread)
 */
TEST(BossSystemTest, FiringEmitsSpreadBullets){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.aimed_fire_interval  = 2;   // 2tick後に発射
    cfg.circle_fire_interval = 999; // 全方向弾は発射させない
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // Entering→Fighting
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // aimed_fire_timer: 2→1
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // aimed_fire_timer: 1→0 → 発射
    EXPECT_GT(enemy_bullets.CountActive(), 0u);
}

/**
 * @brief タイマーが0になるとenemy_bulletsプールに追加されることを確認(EmitCircle)
 */
TEST(BossSystemTest, FiringEmitsCircleBullets){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.aimed_fire_interval  = 999; // 自機狙い弾は発射させない
    cfg.circle_fire_interval = 2;   // 2tick後に発射
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // Entering→Fighting
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
 */
TEST(BossSystemTest, TakeDamageRejectsWhenInactive){
    BossSystem bs;
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.GetBoss().hp, 0);
}

/**
 * @brief damage<=0はInvalidTargetを返すことを確認
 */
TEST(BossSystemTest, TakeDamageRejectsNonPositiveDamage){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    bs.Start(MakeDefaultConfig());
    AdvanceToFighting(bs, enemy_bullets, patterns);
    EXPECT_EQ(bs.TakeDamage(0),  BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.TakeDamage(-1), BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.GetBoss().hp, 10);  // HPは変化しない
}

/**
 * @brief 通常ダメージでDamagedを返しHPが減ることを確認
 */
TEST(BossSystemTest, TakeDamageReducesHp){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    bs.Start(MakeDefaultConfig());
    AdvanceToFighting(bs, enemy_bullets, patterns);
    EXPECT_EQ(bs.TakeDamage(3), BossDamageResult::Damaged);
    EXPECT_EQ(bs.GetBoss().hp, 7);
}

/**
 * @brief HPが0以下になるとDestroyedを返しDefeating遷移することを確認
 */
TEST(BossSystemTest, TakeDamageTransitionsToDefeating){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    bs.Start(MakeDefaultConfig());
    AdvanceToFighting(bs, enemy_bullets, patterns);
    EXPECT_EQ(bs.TakeDamage(10), BossDamageResult::Destroyed);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    EXPECT_TRUE(bs.IsDefeated());
}

/**
 * @brief 超過ダメージ(HPを超える量)でもDestroyedを返すことを確認
 */
TEST(BossSystemTest, TakeDamageOverkillReturnsDestroyed){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    bs.Start(MakeDefaultConfig());
    AdvanceToFighting(bs, enemy_bullets, patterns);
    EXPECT_EQ(bs.TakeDamage(999), BossDamageResult::Destroyed);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
}

// ──────────────────────────────────────────────────────
// 当たり判定円
// ──────────────────────────────────────────────────────
/**
 * @brief GetHurtboxCircle / GetContactCircle がpositionとradiusを正しく返すことを確認
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

// ──────────────────────────────────────────────────────
// [新規テスト] 要求された6項目
// ──────────────────────────────────────────────────────

/**
 * @brief Entering中はTakeDamage()がInvalidTargetを返しHPが変化しないことを確認
 */
TEST(BossSystemTest, Entering_CannotTakeDamage){
    BossSystem bs;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 60;  // 長めに設定してEntering状態を維持
    bs.Start(cfg);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Entering);

    const int hp_before = bs.GetBoss().hp;
    EXPECT_EQ(bs.TakeDamage(3), BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.GetBoss().hp, hp_before);  // HPが減らない
}

/**
 * @brief 同tick内に複数回TakeDamage()を呼ぶとその分だけダメージが蓄積されることを確認
 */
TEST(BossSystemTest, MultiplePlayerBulletsSameTick_AllApplyDamage){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.max_hp = 10;
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);

    // 同tick内に3発ヒット
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::Damaged);
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::Damaged);
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::Damaged);

    EXPECT_EQ(bs.GetBoss().hp, 7);  // 10 - 1*3 = 7
}

/**
 * @brief 撃破後に再びTakeDamage()してもDestroyedが返らないことを確認
 */
TEST(BossSystemTest, DefeatedBoss_CannotTakeFurtherDamage){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.max_hp = 1;
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);

    // 一発で撃破
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::Destroyed);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);

    // 追撃 → Destroyedは返らない
    const auto result = bs.TakeDamage(1);
    EXPECT_NE(result, BossDamageResult::Destroyed);
    EXPECT_EQ(result, BossDamageResult::InvalidTarget);
}

/**
 * @brief entering_duration_ticks ちょうどWaitingTriggerを返し，
 *        続く1tickでActiveに遷移することを確認(tick境界を固定)
 */
TEST(BossSystemTest, EnteringDuration_TransitionsToFightingExactlyOnce){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    constexpr std::uint64_t ENTER_TICKS = 4;
    cfg.entering_duration_ticks = ENTER_TICKS;
    cfg.aimed_fire_interval  = 999;  // 発射させない
    cfg.circle_fire_interval = 999;
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // ENTER_TICKS回分すべてWaitingTriggerが返ること
    for(std::uint64_t i = 0; i < ENTER_TICKS; ++i){
        EXPECT_EQ(
            bs.UpdateTick(dt, player_pos, patterns, enemy_bullets),
            BossEvents::WaitingTrigger
        ) << "tick " << (i + 1);
    }
    // ENTER_TICKS回消費後にFightingへ遷移していること
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // 次のtickはActiveを返す(再度WaitingTriggerには戻らない)
    EXPECT_EQ(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets),
        BossEvents::Active
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Fighting);
}

// ──────────────────────────────────────────────────────
// UpdatePipeline レベルのボス遷移テスト
// ──────────────────────────────────────────────────────

/**
 * @brief BOSS_SPAWN_TICKに達するとボスが一度だけStartされ，
 *        以降はEnemySpawnerも含め二重起動されないことを確認
 */
TEST(UpdatePipelineBossTest, StageBossTransition_StartsOnce){
    UpdatePipeline pipeline;
    InputState no_input{};
    pipeline.StartRun(640.0f, 480.0f, 42);
    const float dt = 1.0f / 60.0f;

    // BOSS_SPAWN_TICK回呼ぶと tick_index_ = BOSS_SPAWN_TICK になる
    // この時点ではまだボスは出現していない
    for(std::uint64_t i = 0; i < zimovka::Stage1::BOSS_SPAWN_TICK; ++i){
        pipeline.UpdateTick(dt, no_input);
    }
    EXPECT_FALSE(pipeline.GetBossSystem().IsNotInactive());

    // もう1tickでボス出現(tick_index_ == BOSS_SPAWN_TICK で StartBossPhase()が呼ばれる)
    const std::size_t spawner_idx_at_start = pipeline.GetSpawnerNextEventIndex();
    pipeline.UpdateTick(dt, no_input);
    EXPECT_TRUE(pipeline.GetBossSystem().IsNotInactive());

    // 出現直後のmax_hpを記録
    const int boss_max_hp = pipeline.GetBossSystem().GetBoss().max_hp;
    const std::size_t spawner_idx_after_boss = pipeline.GetSpawnerNextEventIndex();

    // さらに10tick進めてもボスは再Startされていない
    for(int i = 0; i < 10; ++i){
        pipeline.UpdateTick(dt, no_input);
    }
    EXPECT_TRUE(pipeline.GetBossSystem().IsNotInactive());
    EXPECT_EQ(pipeline.GetBossSystem().GetBoss().max_hp, boss_max_hp);

    // EnemySpawnerは Boss フェーズでは呼ばれないためインデックスが凍結している
    EXPECT_EQ(pipeline.GetSpawnerNextEventIndex(), spawner_idx_after_boss);

    // 念のため: Boss出現直後にSpawnerが余分に前進していないことを確認
    // (BOSS_SPAWN_TICK時点のSpawnerは全Stage1イベント処理済み=30, 増えていない)
    EXPECT_EQ(spawner_idx_at_start, spawner_idx_after_boss);
}
