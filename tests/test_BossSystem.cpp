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

// Entering → Fightingへの進行ヘルパ
static void AdvanceToFighting(
    BossSystem& bs, BulletSystem& bullets, PatternSystem& patterns
)
{
    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // duration_tickの消費
    while(bs.GetBoss().status == BossStatus::Entering){
        bs.UpdateTick(dt, player_pos, patterns, bullets);
    }
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
 * @brief 有効なConfigでStart()が例外を投げない
 * 
 */
TEST(BossSystemTest, StartSucceedsWithValidConfig){
    BossSystem bs;
    EXPECT_NO_THROW(bs.Start(MakeDefaultConfig()));
}

/**
 * @brief nameが空のConfig
 * 
 */
TEST(BossSystemTest, StartRejectsEmptyName){
    BossSystem bs;
    BossConfig cfg = MakeDefaultConfig();
    cfg.name = "";
    EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
}

/**
 * @brief Vec2フィールドが無限大のConfig
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
 * @brief 数値フィールドが不正なConfig
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

/**
 * @brief Inactive以外の状態でStart()を呼ぶとlogic_errorが投げられることを確認
 */
TEST(BossSystemTest, Start_WhenNotInactive_ThrowsLogicError){
    BossSystem bs;
    bs.Start(MakeDefaultConfig());
    // Entering状態でもう一度Start() → logic_error
    EXPECT_THROW(bs.Start(MakeDefaultConfig()), std::logic_error);
}

// ──────────────────────────────────────────────────────
// Start()後の状態
// ──────────────────────────────────────────────────────
/**
 * @brief Start()後はEntering状態になるか
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
 * @brief Start()後はConfigの値がBossへ設定される
 * 
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
 * @brief 撃破後にReset()するとInactive・HP・タイマーがすべて初期化される
 * 
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

    // Resetすると初期状態になっている
    bs.Reset();
    const auto& boss = bs.GetBoss();
    EXPECT_EQ(boss.status, BossStatus::Inactive);
    EXPECT_FALSE(bs.IsNotInactive());
    EXPECT_EQ(boss.hp, 0);               // デフォルト値(0)に戻る
    EXPECT_EQ(boss.aimed_fire_timer,  0u);
    EXPECT_EQ(boss.circle_fire_timer, 0u);
}

// ──────────────────────────────────────────────────────
// UpdateTick()状態遷移
// ──────────────────────────────────────────────────────
/**
 * @brief Inactive時はNoneを返す
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
 * @brief Entering中はWaitingTriggerを返す
 * 
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
 * @brief entering_duration_ticks消費でEntering→Fightingへの遷移を確認
 * 
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
 * @brief Fighting中はActiveを返す
 * 
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
 * @brief Defeating状態はDefeatedを返す
 * 
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
// UpdateTick()弾発射
// ──────────────────────────────────────────────────────
/**
 * @brief タイマーが0になるとenemy_bulletsプールに追加されることを確認
 * ※EmitSpread
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

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // aimed_fire_timer: 1→0 → 1回目発射
    const std::size_t count_after_shot1 = enemy_bullets.CountActive();
    EXPECT_GT(count_after_shot1, 0u);   // 少なくとも1発出ている

    // 2回目の発射(aimed_fire_intervalの分だけ進める)
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // aimed_fire_timer: 2→1
    EXPECT_EQ(enemy_bullets.CountActive(), count_after_shot1); // まだ増えない
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // aimed_fire_timer: 1→0 → 2回目発射
    EXPECT_EQ(enemy_bullets.CountActive(), count_after_shot1 * 2u); // 発射回数2回分
}

/**
 * @brief タイマーが0になるとenemy_bulletsプールに追加されることを確認
 * ※EmitCircle
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

    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // circle_fire_timer: 1→0 → 1回目発射
    const std::size_t count_after_shot1 = enemy_bullets.CountActive();
    EXPECT_GT(count_after_shot1, 0u);   // 少なくとも1発出ている

    // 2回目の発射(circle_fire_intervalの分だけ進める)
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // circle_fire_timer: 2→1
    EXPECT_EQ(enemy_bullets.CountActive(), count_after_shot1); // まだ増えない
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); // circle_fire_timer: 1→0 → 2回目発射
    EXPECT_EQ(enemy_bullets.CountActive(), count_after_shot1 * 2u); // 発射回数2回分
}

// ──────────────────────────────────────────────────────
// TakeDamage()
// ──────────────────────────────────────────────────────
/**
 * @brief Inactive時はInvalidTargetを返す
 * 
 */
TEST(BossSystemTest, TakeDamageRejectsWhenInactive){
    BossSystem bs;
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.GetBoss().hp, 0);
}

/**
 * @brief damage<=0はInvalidTargetを返す
 * 
 */
TEST(BossSystemTest, TakeDamageRejectsNonPositiveDamage){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    // Entering→fighting
    bs.Start(MakeDefaultConfig());
    AdvanceToFighting(bs, enemy_bullets, patterns);
    // 無効なダメージ
    EXPECT_EQ(bs.TakeDamage(0),  BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.TakeDamage(-1), BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.GetBoss().hp, 10);  // HPは変化しない
}

/**
 * @brief 通常ダメージでDamagedを返しHPが減る
 * 
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
 * @brief HPが0以下になるとDestroyedを返しDefeating遷移する
 * 
 */
TEST(BossSystemTest, TakeDamageTransitionsToDefeating){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    // fighting
    bs.Start(MakeDefaultConfig());
    AdvanceToFighting(bs, enemy_bullets, patterns);
    // max_hp=10に対して10ダメージ与える→Destroyed
    EXPECT_EQ(bs.TakeDamage(10), BossDamageResult::Destroyed);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    EXPECT_TRUE(bs.IsDefeated());
}

/**
 * @brief 超過ダメージ(HPを超える量)でもDestroyedを返す
 * 
 */
TEST(BossSystemTest, TakeDamageOverkillReturnsDestroyed){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    bs.Start(MakeDefaultConfig());
    AdvanceToFighting(bs, enemy_bullets, patterns);
    // max_hp=10に対して999のダメージ
    EXPECT_EQ(bs.TakeDamage(999), BossDamageResult::Destroyed);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
}

// ──────────────────────────────────────────────────────
// 当たり判定円
// ──────────────────────────────────────────────────────
/**
 * @brief GetHurtboxCircle/GetContactCircleがpositionとradiusを正しく返すか
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

// ──────────────────────────────────────────────────────
// 追加テスト
// ──────────────────────────────────────────────────────

/**
 * @brief Entering中はTakeDamage()がInvalidTargetを返しHPが変化しない
 * 
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
 * @brief 同tick内に複数回TakeDamage()を呼ぶとその分だけダメージが蓄積されるか
 * 
 */
TEST(BossSystemTest, MultiplePlayerBulletsSameTick_AllApplyDamage){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.max_hp = 10;
    // fighting
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);

    // 同tick内に3発ヒット
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::Damaged);
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::Damaged);
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::Damaged);

    EXPECT_EQ(bs.GetBoss().hp, 7);  // 10 - 1*3 = 7
}

/**
 * @brief 撃破後に再びTakeDamage()してもDestroyedが返らない
 * 
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
 * @brief entering_duration_ticks：1→0の境界でWaitingTriggerを返し，
 * 続く1tickでActiveに遷移することを確認(tick境界を固定)
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

    // ENTER_TICKS回分すべてWaitingTriggerが返る
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
 * 以降はEnemySpawnerも含め二重起動されない
 */
TEST(UpdatePipelineBossTest, StageBossTransition_StartsOnce){
    UpdatePipeline pipeline;
    InputState no_input{};
    pipeline.StartRun(640.0f, 720.0f, 42);
    const float dt = 1.0f / 60.0f;

    // BOSS_SPAWN_TICK回呼ぶとtick_index_ = BOSS_SPAWN_TICKになる
    // この時点ではまだボスは出現しない
    for(std::uint64_t i = 0; i < zimovka::Stage1::BOSS_SPAWN_TICK; ++i){
        pipeline.UpdateTick(dt, no_input);
    }
    EXPECT_FALSE(pipeline.GetBossSystem().IsNotInactive());

    // もう1tickでボス出現(StartBossPhase()が呼ばれる)
    pipeline.UpdateTick(dt, no_input);
    EXPECT_TRUE(pipeline.GetBossSystem().IsNotInactive());

    // ボス出現直後のSpawnerカーソルを記録
    const std::size_t spawner_idx_after_boss = pipeline.GetSpawnerNextEventIndex();

    // さらに10tick進める
    for(int i = 0; i < 10; ++i){
        pipeline.UpdateTick(dt, no_input);
    }
    // ボスはまだActive
    EXPECT_TRUE(pipeline.GetBossSystem().IsNotInactive());
    // EnemySpawnerはBossフェーズでは呼ばれないためカーソルが凍結している
    EXPECT_EQ(pipeline.GetSpawnerNextEventIndex(), spawner_idx_after_boss);
}
