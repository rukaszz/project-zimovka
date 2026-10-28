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
#include "zimovka/events/BossTickEvents.hpp"
#include "zimovka/input/Action.hpp"
#include "zimovka/stages/StagePhase.hpp"
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
using zimovka::BossTickEvents;
using zimovka::BulletSystem;
using zimovka::InputState;
using zimovka::PatternSystem;
using zimovka::StageStatement;
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
    EXPECT_FALSE(bs.IsDefeating());
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
 * @brief Inactive時はFalseを返す
 * 
 */
TEST(BossSystemTest, UpdateTickReturnsFalseWhenInactive){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_FALSE(
        bs.UpdateTick(1.0f/60.0f, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Inactive);
}

/**
 * @brief Entering中，UpdateTickはfalseを返す
 * 
 */
TEST(BossSystemTest, UpdateTickReturnsFalseDuringEntering){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 60;    // 十分な長さに設定
    bs.Start(cfg);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Entering);

    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_FALSE(
        bs.UpdateTick(1.0f/60.0f, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Entering);
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
 * @brief Fighting中，UpdateTickはfalseを返す
 * 
 */
TEST(BossSystemTest, UpdateTickReturnsFalseWhenFighting){
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
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Fighting);
}

/**
 * @brief Defeating状態はDefeatedを返す
 * 
 */
TEST(BossSystemTest, UpdateTickReturnsFalseWhenDefeating){
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
    EXPECT_TRUE(bs.IsDefeating());

    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_FALSE(
        bs.UpdateTick(1.0f/60.0f, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
}

/**
 * @brief 敗北演出完了時，UpdateTickはfalseを返す
 * 
 */
TEST(BossSystemTest, UpdateTickReturnsFalseWhenDefeating){
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
    EXPECT_TRUE(bs.IsDefeating());
    // Defeating中
    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_FALSE(
        bs.UpdateTick(1.0f/60.0f, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);

    // 敗北演出完了時
    EXPECT_TRUE(
        bs.UpdateTick(1.0f/60.0f, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
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
    EXPECT_TRUE(bs.IsDefeating());
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
// 当たり判定
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
// 追加テスト: ボスの状態遷移関係
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
    cfg.max_hp = 1; // HPを1で設定
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
 * 次の1tickでActiveに遷移することを確認(tickの境界を確認)
 * 
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

    // ENTER_TICKS回分すべてdefeated_animation_completed=falseかつEntering状態
    for(std::uint64_t i = 0; i < ENTER_TICKS; ++i){
        EXPECT_FALSE(
            bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
        ) << "tick " << (i + 1);
    }
    // ENTER_TICKS回消費後にFightingへ遷移していること
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // 次のtickもdefeated_animation_completed=false, Fightingのまま
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Fighting);
}

// ──────────────────────────────────────────────────────
// 撃破演出 (Defeating state countdown)
// ──────────────────────────────────────────────────────
/**
 * @brief Defeating状態でUpdateTickを呼ぶごとにdefeating_ticks_remainingが1ずつ減るか
 * 
 */
TEST(BossSystemTest, Defeating_DecrementsTicksEachUpdate){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.max_hp = 1;
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);  // HP→0, Defeating遷移
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    // defeating_ticks_remainingを記録
    const std::uint64_t initial = bs.GetBoss().defeating_ticks_remaining;
    ASSERT_GT(initial, 0u);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // defeating_tickが-1される
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets); 
    EXPECT_EQ(bs.GetBoss().defeating_ticks_remaining, initial - 1u);
    // defeating_tickが-1され合計-2
    bs.UpdateTick(dt, player_pos, patterns, enemy_bullets);
    EXPECT_EQ(bs.GetBoss().defeating_ticks_remaining, initial - 2u);
}

/**
 * @brief defeating_ticks_remaining > 0の間はDefeatedイベントを返すことを確認
 * 
 */
TEST(BossSystemTest, Defeating_ReturnsDefeatedWhileTicksRemaining){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.max_hp = 1;
    // 1ダメージで撃破
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // remaining > 0 の間は defeat_animation_completed = false
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_GT(bs.GetBoss().defeating_ticks_remaining, 0u);
}

/**
 * @brief defeating_ticks_remaining==0でDefeatAnimationCompleteを返すか
 * 
 */
TEST(BossSystemTest, Defeating_ReturnsAnimationCompleteAtZero){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks  = 1;
    cfg.max_hp                   = 1;
    cfg.aimed_fire_interval      = 999;
    cfg.circle_fire_interval     = 999;
    // 演出時間は短く設定
    const std::uint64_t DEFEAT_TICKS = 3;
    cfg.defeating_duration_ticks = DEFEAT_TICKS;
    bs.Start(cfg);
    // 1ダメージを与え撃破
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);
    ASSERT_EQ(bs.GetBoss().defeating_ticks_remaining, DEFEAT_TICKS);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // (DEFEAT_TICKS-1)回は defeat_animation_completed = false
    for(std::uint64_t i = 0; i < DEFEAT_TICKS - 1; ++i){
        EXPECT_FALSE(
            bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
        ) << "tick " << i;
    }
    // DEFEAT_TICKS回目のtickでremainingが0になりdefeat_animation_completed = true
    EXPECT_EQ(bs.GetBoss().defeating_ticks_remaining, 1u);
    EXPECT_TRUE(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().defeating_ticks_remaining, 0u);
}

// ──────────────────────────────────────────────────────
// UpdatePipeline レベルのボス遷移テスト
// ──────────────────────────────────────────────────────
/**
 * @brief BOSS_SPAWN_TICKに達するとボスが一度だけStartされ，
 * 以降はEnemySpawnerも含め二重起動されない
 * 
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

// ──────────────────────────────────────────────────────
// BossSystem: 撃破演出の完了タイミング
// ──────────────────────────────────────────────────────
/**
 * @brief defeating_duration_ticksで指定したtick数で演出が完了することを確認
 *
 * BossSystem::UpdateTick()をdefeating_duration_ticks回呼んだ最後のtickで
 * defeat_animation_completed = true，BossState::Inactiveに遷移
 */
TEST(BossSystemTest, Defeating_CompletesAtExactTick){
    // 設定
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks  = 1;
    cfg.max_hp                   = 1;
    cfg.aimed_fire_interval      = 999;
    cfg.circle_fire_interval     = 999;
    const std::uint64_t DEFEAT_TICKS = 5;
    cfg.defeating_duration_ticks = DEFEAT_TICKS;
    // ボス戦→敗北
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    ASSERT_EQ(bs.GetBoss().defeating_ticks_remaining, DEFEAT_TICKS);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // (DEFEAT_TICKS-1)回まで消費→defeat_animation_completedはfalse
    for(std::uint64_t i = 0; i < DEFEAT_TICKS - 1; ++i){
        EXPECT_FALSE(
            bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
        ) << "tick " << i;
        EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    }
    // DEFEAT_TICKS回目で完了のタイミングでdefeat_animation_completed=true, BossStatus::Inactiveへ遷移
    const BossTickEvents last = bs.UpdateTick(dt, player_pos, patterns, enemy_bullets);
    EXPECT_TRUE(last.defeat_animation_completed);
    EXPECT_EQ(bs.GetBoss().defeating_ticks_remaining, 0u);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Inactive);
}

/**
 * @brief defeat_animation_completedは演出完了tickに1度だけ立つことを確認
 *
 * ※完了後はボスがInactiveになるためUpdateTickは空のイベントを返す
 */
TEST(BossSystemTest, DefeatAnimationComplete_EmittedOnce){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks  = 1;
    cfg.max_hp                   = 1;
    cfg.defeating_duration_ticks = 1;   // 1tickで即完了
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // 1tick目: 完了通知
    EXPECT_TRUE(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Inactive);
    // 2tick目以降: Inactiveなので空のイベント
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, patterns, enemy_bullets).defeat_animation_completed
    );
}

// ──────────────────────────────────────────────────────
// UpdatePipeline: 撃破演出・ステージクリアの状態遷移
// ──────────────────────────────────────────────────────
namespace{
/**
 * @brief Normal → Boss(defeating) まで進めるヘルパ
 *
 * BOSS_SPAWN_TICK まで進め，SHOOTを押し続けてボスを撃破する。
 * 撃破イベント(defeated_boss)が発火したtickで停止する。
 * @return true  撃破成功
 * @return false タイムアウト
 */
bool AdvancePipelineToDefeating(UpdatePipeline& pipeline){
    const float dt = 1.0f / 60.0f;
    InputState no_input{};
    const std::uint32_t shoot_bit = zimovka::ActionBit(zimovka::Action::Shoot);
    InputState shoot_input = InputState::FromBits(shoot_bit, shoot_bit, 0);
    // Normal フェーズ: BOSS_SPAWN_TICK まで進める
    for(std::uint64_t i = 0; i <= zimovka::Stage1::BOSS_SPAWN_TICK; ++i){
        pipeline.UpdateTick(dt, no_input);
    }
    // Boss フェーズ: SHOOTを押し続けてボスを撃破
    constexpr std::uint64_t TIMEOUT = 5000;
    for(std::uint64_t i = 0; i < TIMEOUT; ++i){
        const auto ev = pipeline.UpdateTick(dt, shoot_input);
        if(ev.boss_hit.defeated_boss){
            return true;
        }
    }
    return false;
}

/**
 * @brief Normal → Cleared まで進めるヘルパ
 *
 * AdvancePipelineToDefeating 後に撃破演出が完了するまで続ける。
 * @return true  クリア成功
 * @return false タイムアウト
 */
bool AdvancePipelineToCleared(UpdatePipeline& pipeline){
    const float dt = 1.0f / 60.0f;
    InputState no_input{};
    const std::uint32_t shoot_bit = zimovka::ActionBit(zimovka::Action::Shoot);
    InputState shoot_input = InputState::FromBits(shoot_bit, shoot_bit, 0);
    // Normal フェーズ
    for(std::uint64_t i = 0; i <= zimovka::Stage1::BOSS_SPAWN_TICK; ++i){
        pipeline.UpdateTick(dt, no_input);
    }
    // Boss フェーズ → Cleared まで
    constexpr std::uint64_t TIMEOUT = 5500;
    for(std::uint64_t i = 0; i < TIMEOUT; ++i){
        const auto ev = pipeline.UpdateTick(dt, shoot_input);
        if(ev.stage_clear_started){
            return true;
        }
    }
    return false;
}
}   // anonymous namespace

/**
 * @brief 撃破演出中はPlayer・Weapon・Bulletの戦闘更新が行われないことを確認
 *
 * Defeating中にUpdateTickを呼んでも
 * →移動入力を渡してもプレイヤー位置が変わらない
 * →SHOOT入力を渡しても自機弾が増えない
 */
TEST(UpdatePipelineBossTest, Defeating_StopsCombatUpdates){
    UpdatePipeline pipeline;
    pipeline.StartRun(640.0f, 720.0f, 42);
    ASSERT_TRUE(AdvancePipelineToDefeating(pipeline));
    // この時点でボスはDefeating状態
    ASSERT_TRUE(pipeline.GetBossSystem().IsDefeating());

    const float dt = 1.0f / 60.0f;
    // 撃破直後に敵弾・自機弾がClearされる
    EXPECT_EQ(pipeline.GetPlayerBullets().CountActive(), 0u);
    EXPECT_EQ(pipeline.GetEnemyBullets().CountActive(), 0u);

    // 移動 + SHOOTを渡しても状態は変化しない
    InputState move_shoot = InputState::FromBits(
        zimovka::ActionBit(zimovka::Action::MoveLeft)
        | zimovka::ActionBit(zimovka::Action::Shoot),
        0, 0
    );
    const Vec2 pos_before = pipeline.GetPlayerSystem().GetPlayerPosition();
    pipeline.UpdateTick(dt, move_shoot);
    const Vec2 pos_after  = pipeline.GetPlayerSystem().GetPlayerPosition();
    EXPECT_EQ(pos_before.x, pos_after.x);
    EXPECT_EQ(pos_before.y, pos_after.y);
    // SHOOT 入力があっても自機弾は0のまま
    EXPECT_EQ(pipeline.GetPlayerBullets().CountActive(), 0u);
}

/**
 * @brief ステージクリア後も戦闘更新が再開しないことを確認
 *
 * Cleared状態に入ると:
 * stage_clear_ticks_elapsed がtick毎にインクリメントされる
 * 移動入力・射撃入力を渡してもプレイヤー位置・弾数に変化がない
 */
TEST(UpdatePipelineBossTest, Cleared_StopsCombatUpdates){
    UpdatePipeline pipeline;
    pipeline.StartRun(640.0f, 720.0f, 42);
    ASSERT_TRUE(AdvancePipelineToCleared(pipeline));
    ASSERT_EQ(pipeline.GetStagePhase().stage_state, StageStatement::Cleared);

    const float dt = 1.0f / 60.0f;
    InputState move_shoot = InputState::FromBits(
        zimovka::ActionBit(zimovka::Action::MoveLeft)
        | zimovka::ActionBit(zimovka::Action::Shoot),
        0, 0
    );
    // クリア直後のカウンタ
    const std::uint64_t elapsed_before = pipeline.GetStagePhase().stage_clear_ticks_elapsed;
    const Vec2 pos_before = pipeline.GetPlayerSystem().GetPlayerPosition();

    // 3tick進める
    for(int i = 0; i < 3; ++i){
        pipeline.UpdateTick(dt, move_shoot);
    }
    // stage_clear_ticks_elapsed が 3 増えている
    EXPECT_EQ(pipeline.GetStagePhase().stage_clear_ticks_elapsed, elapsed_before + 3u);
    // プレイヤー位置は変わっていない
    const Vec2 pos_after = pipeline.GetPlayerSystem().GetPlayerPosition();
    EXPECT_EQ(pos_before.x, pos_after.x);
    EXPECT_EQ(pos_before.y, pos_after.y);
    // 自機弾も増えていない
    EXPECT_EQ(pipeline.GetPlayerBullets().CountActive(), 0u);
}

/**
 * @brief 同一のUpdatePipelineでStartRunを再実行するとNormal状態から再スタートできることを確認
 *
 * ステージクリア後に再実行した場合でも初期状態に戻る。
 */
TEST(UpdatePipelineBossTest, StartRun_ResetsStageState){
    UpdatePipeline pipeline;
    pipeline.StartRun(640.0f, 720.0f, 42);
    ASSERT_TRUE(AdvancePipelineToCleared(pipeline));
    ASSERT_EQ(pipeline.GetStagePhase().stage_state, StageStatement::Cleared);

    // 再実行
    pipeline.StartRun(640.0f, 720.0f, 42);
    EXPECT_EQ(pipeline.GetStagePhase().stage_state, StageStatement::Normal);
    EXPECT_EQ(pipeline.GetStagePhase().stage_clear_ticks_elapsed, 0u);
    EXPECT_FALSE(pipeline.GetBossSystem().IsNotInactive());
    EXPECT_EQ(pipeline.GetTickIndex(), 0u);
}
