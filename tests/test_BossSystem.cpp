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
#include "zimovka/systems/boss/BossAttackSequenceDef.hpp"
#include "zimovka/systems/boss/BossAttackStep.hpp"
#include "zimovka/systems/boss/BossConfig.hpp"
#include "zimovka/systems/boss/BossPhaseDef.hpp"
#include "zimovka/events/BossTickEvents.hpp"
#include "zimovka/input/Action.hpp"
#include "zimovka/stages/StagePhase.hpp"
#include "zimovka/systems/boss/BossDamageResult.hpp"
#include "zimovka/systems/boss/BossEvents.hpp"
#include "zimovka/systems/boss/BossStatus.hpp"
#include "zimovka/systems/boss/BossSystem.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/pattern/PatternSystem.hpp"

using zimovka::BossAttackSequenceDef;
using zimovka::BossAttackStep;
using zimovka::BossConfig;
using zimovka::BossMovementDef;
using zimovka::BossMoveTarget;
using zimovka::BossMoveType;
using zimovka::BossPhaseKind;
using zimovka::BossShotDef;
using zimovka::BossPhaseDef;
using zimovka::BossDamageResult;
using zimovka::BossEvents;
using zimovka::BossStatus;
using zimovka::BossSystem;
using zimovka::BossTickEvents;
using zimovka::BulletSystem;
using zimovka::DeterministicRng;
using zimovka::InputState;
using zimovka::PatternSystem;
using zimovka::StageStatement;
using zimovka::UpdatePipeline;
using zimovka::Vec2;

namespace{
// テスト用RNG(テスト設定ではRandomモードを使わないため共有しても問題ない)
DeterministicRng& GetTestRng(){
    static DeterministicRng rng{0};
    return rng;
}
}  // namespace

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
    cfg.entering_duration_ticks  = 3;    // テスト用に短く設定
    // 単一フェーズ(max_hp=10, clear_at_hp=0)で単純なテストに適した構成
    cfg.phases = {{
        BossPhaseKind::Normal, 10, 0,
        {{BossAttackStep::AimedPressure, 999, BossShotDef{3, 5, 100.0f, 3.0f, 0.5f}}}
    }};
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
    const auto enter_ticks = bs.GetBoss().entering_ticks_remaining;
    for(std::uint64_t i = 0;i < enter_ticks; ++i){
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    }
    ASSERT_EQ(
        bs.GetBoss().status,
        BossStatus::Fighting
    );
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
    {   // phases[0].max_hp=0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].max_hp = 0;
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
    {   // 発射ステップにBossShotDefがない
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{BossAttackStep::AimedPressure, 10}};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // attack_sequence が空
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence.clear();
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // attack_sequence の duration_ticks = 0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{BossAttackStep::AimedPressure, 0}};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // attack_sequence に不正な step (Count 以上)
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{BossAttackStep::Count, 60}};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
}

/**
 * @brief 空のattack_sequenceを渡すとStart()がstd::invalid_argumentを投げることを確認
 *
 * 契約: attack_sequenceは少なくとも1要素を持たなければならない
 */
TEST(BossSystemTest, Start_RejectsEmptyAttackSequence){
    BossSystem bs;
    BossConfig cfg = MakeDefaultConfig();
    cfg.phases[0].attack_sequence.clear();
    EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
}

/**
 * @brief duration_ticks = 0のステップを含むsequenceを渡すとStart()が例外を投げることを確認
 *
 * 契約: 全てのステップのduration_ticksは1以上でなければならない
 * 先頭ステップだけでなく途中のステップも検証される
 */
TEST(BossSystemTest, Start_RejectsZeroStepDuration){
    {   // 先頭ステップが 0
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{BossAttackStep::AimedPressure, 0}};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // 2番目のステップが 0 (validation は全ステップを走査する)
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {
            {BossAttackStep::AimedPressure, 60},
            {BossAttackStep::CircleBarrage,  0},
        };
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
}

/**
 * @brief 無効なBossAttackStep値を含むsequenceを渡すとStart()が例外を投げることを確認
 *
 * 契約: 有効な範囲は[AimedPressure, Count)
 * Count (番兵値)，Countより大きい値，負の値はいずれも拒否される
 */
TEST(BossSystemTest, Start_RejectsInvalidAttackStep){
    {   // Count(番兵値) = 無効
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{BossAttackStep::Count, 60}};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // Countより大きい値
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{static_cast<BossAttackStep>(127), 60}};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    {   // AimedPressure(0)より小さい値(負数)
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{static_cast<BossAttackStep>(-1), 60}};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
}

/**
 * @brief Inactive以外の状態でStart()を呼ぶとlogic_errorが投げられることを確認
 * 
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
    EXPECT_FLOAT_EQ(boss.hurtbox_radius, 30.0f);
    EXPECT_FLOAT_EQ(boss.contact_radius, 20.0f);
}

/**
 * @brief Start()直後のattack_stepとattack_step_ticks_remainingが
 *        attack_sequence[0]の定義と一致することを確認
 *
 * 契約: EnterAttackStep(0)により初期状態は必ずsequence[0]から始まる．
 *       AimedPressure以外を先頭に置いて確認することで
 *       旧来のハードコードではなくsequence定義が使われていることを保証する．
 */
TEST(BossSystemTest, Start_UsesFirstSequenceDefinition){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::CircleBarrage, 120, BossShotDef{12, 60, 150.0f, 3.0f, 0.0f}},  // 先頭が CircleBarrage
        {BossAttackStep::AimedPressure,  60, BossShotDef{3, 5, 100.0f, 3.0f, 0.5f}},
    };
    bs.Start(cfg);
    // EnterAttackStep(0)はEntering→Fighting遷移時に呼ばれる
    AdvanceToFighting(bs, enemy_bullets, patterns);
    const auto& boss = bs.GetBoss();
    EXPECT_EQ(boss.attack_step, BossAttackStep::CircleBarrage);
    EXPECT_EQ(boss.attack_step_ticks_remaining, 120u);
    EXPECT_EQ(boss.attack_step_index,           0u);
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
    bs.TakeDamage(bs.GetBoss().hp);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);

    // Resetすると初期状態になっている
    bs.Reset();
    const auto& boss = bs.GetBoss();
    EXPECT_EQ(boss.status, BossStatus::Inactive);
    EXPECT_FALSE(bs.IsNotInactive());
    EXPECT_EQ(boss.hp, 0);               // デフォルト値(0)に戻る
    EXPECT_EQ(boss.fire_timer, 0u);
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
        bs.UpdateTick(1.0f/60.0f, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
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
        bs.UpdateTick(1.0f/60.0f, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
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

    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);

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
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // Entering→Fighting
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Fighting);
}

/**
 * @brief Defeating状態はdefeating_duration_ticksを消費するまでfalseを返す
 * 
 */
TEST(BossSystemTest, UpdateTickReturnsFalseWhenDefeating){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].max_hp = 1;
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);  // HP→0, Defeating遷移
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    EXPECT_TRUE(bs.IsDefeating());

    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_FALSE(
        bs.UpdateTick(1.0f/60.0f, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
}

/**
 * @brief 敗北演出完了時，UpdateTickはtrueを返す
 * 
 */
TEST(BossSystemTest, UpdateTickReturnsTrueWhenTicksConsumed){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.defeating_duration_ticks = 2;
    cfg.phases[0].max_hp = 1;
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);  // HP→0, Defeating遷移
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    EXPECT_TRUE(bs.IsDefeating());
    // Defeating中
    const Vec2 player_pos{320.0f, 400.0f};
    EXPECT_FALSE(
        bs.UpdateTick(1.0f/60.0f, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);

    // 敗北演出完了時
    EXPECT_TRUE(
        bs.UpdateTick(1.0f/60.0f, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Inactive);
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
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure, 999, BossShotDef{3, 2, 100.0f, 3.0f, 0.5f}},
    };
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // Entering→Fighting
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // fire_timer: 2→1
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // fire_timer: 1→0 → 1回目発射
    const std::size_t count_after_shot1 = enemy_bullets.CountActive();
    EXPECT_GT(count_after_shot1, 0u);   // 少なくとも1発出ている

    // 2回目の発射(fire_interval_tickの分だけ進める)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);         // fire_timer: 2→1
    EXPECT_EQ(enemy_bullets.CountActive(), count_after_shot1);      // まだ増えない
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);         // fire_timer: 1→0 → 2回目発射
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
    cfg.phases[0].max_hp = 10;
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
    cfg.phases[0].max_hp = 1; // HPを1で設定
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
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // ENTER_TICKS回分すべてdefeated_animation_completed=falseかつEntering状態
    for(std::uint64_t i = 0; i < ENTER_TICKS; ++i){
        EXPECT_FALSE(
            bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
        ) << "tick " << (i + 1);
    }
    // ENTER_TICKS回消費後にFightingへ遷移していること
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // 次のtickもdefeated_animation_completed=false, Fightingのまま
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
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
    cfg.phases[0].max_hp = 1;
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
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); 
    EXPECT_EQ(bs.GetBoss().defeating_ticks_remaining, initial - 1u);
    // defeating_tickが-1され合計-2
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
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
    cfg.phases[0].max_hp = 1;
    // 1ダメージで撃破
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // remaining > 0 の間は defeat_animation_completed = false
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
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
    cfg.phases[0].max_hp         = 1;
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
            bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
        ) << "tick " << i;
    }
    // DEFEAT_TICKS回目のtickでremainingが0になりdefeat_animation_completed = true
    EXPECT_EQ(bs.GetBoss().defeating_ticks_remaining, 1u);
    EXPECT_TRUE(
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
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
    cfg.phases[0].max_hp         = 1;
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
            bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
        ) << "tick " << i;
        EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    }
    // DEFEAT_TICKS回目で完了のタイミングでdefeat_animation_completed=true, BossStatus::Inactiveへ遷移
    const BossTickEvents last = bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
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
    cfg.phases[0].max_hp         = 1;
    cfg.defeating_duration_ticks = 1;   // 1tickで即完了
    bs.Start(cfg);
    AdvanceToFighting(bs, enemy_bullets, patterns);
    bs.TakeDamage(1);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // 1tick目: 完了通知
    EXPECT_TRUE(
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Inactive);
    // 2tick目以降: Inactiveなので空のイベント
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
    );
    EXPECT_FALSE(
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets).defeat_animation_completed
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

// ──────────────────────────────────────────────────────
// BossAttackSequence 状態遷移契約テスト
// ──────────────────────────────────────────────────────
// 
// attack_sequence: [{AimedPressure,3}, {CircleBarrage,4}]
// 発射タイマーを抑制してステップ遷移のみに集中する

/**
 * @brief 攻撃遷移テスト用の短い2ステップシーケンス設定ヘルパ
 * 
 * entering_duration_ticks=1(AimedPressure初回)
 * 
 * @return BossConfig 
 */
static BossConfig MakeSequenceConfig(){
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks      = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure, 3, BossShotDef{3, 999, 100.0f, 3.0f, 0.0f}},
        {BossAttackStep::CircleBarrage, 4, BossShotDef{3, 999, 100.0f, 3.0f, 0.0f}},
    };
    return cfg;
}

/**
 * @brief 各攻撃ステップがconfigで指定したduration_ticks分だけ継続することを確認
 *
 * 契約:
 *   - attack_step_ticks_remaining == 0になった次のUpdateFighting呼び出しで
 *     AdvanceAttackStep()が呼ばれ, 新しいステップへ遷移する
 *   - attack_sequence[index].duration_ticksのtick分継続する
 *
 * シーケンス: [{AimedPressure,3}, {CircleBarrage,4}]
 * 初回のAimedPressure durationは3tick
 *
 * Fighting tick 1-3: AimedPressure中
 * Fighting tick 4  : remaining==0 → CircleBarrageへ遷移する
 * Fighting tick 4-7: CircleBarrage中
 * Fighting tick 8  : remaining==0 → AimedPressureへwrap-around
 */
TEST(BossAttackSequenceTest, AttackSequence_TransitionsAtExactTicks){
    // ボスの設定
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeSequenceConfig();
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting(1tick)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // Fighting tick 1–3: AimedPressure中
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 3→2
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::AimedPressure);
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 2→1
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::AimedPressure);
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 1→0
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::AimedPressure);

    // Fighting tick 4: remaining==0でAdvanceAttackStep → CircleBarrage
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::CircleBarrage);

    // Fighting tick 5–7: CircleBarrage中(duration=4 → remaining 3,2,1,0)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 3→2
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::CircleBarrage);
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 2→1
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::CircleBarrage);
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 1→0
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::CircleBarrage);

    // Fighting tick 8: remaining==0でwrap-around → AimedPressure(index 0, duration=3)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::AimedPressure);
}

/**
 * @brief AimedPressureステップ中は自機狙い弾のみ発射されることを確認
 *
 * 契約:
 *   - AimedPressureステップではUpdateAimedAttack()のみ呼ばれる
 *   - 1発目と2発目で増加する弾数が等しい(毎回同じ弾数を発射)
 *
 * fire_interval_tick=2のBossShotDefを設定し,
 * AimedPressureのみのシーケンスで動作させる.
 */
TEST(BossAttackSequenceTest, AimedPressure_EmitsSpreadOnly){
    BossSystem bs;
    BulletSystem enemy_bullets(200);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks    = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure, 999, BossShotDef{3, 2, 100.0f, 3.0f, 0.5f}},
    };
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    // fire_timer: 2→1(まだ発射されない)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(enemy_bullets.CountActive(), 0u);

    // fire_timer: 1→0 → 1回目発射
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    const std::size_t shot1 = enemy_bullets.CountActive();
    EXPECT_GT(shot1, 0u);

    // fire_timer: 2→1(弾は増えない)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(enemy_bullets.CountActive(), shot1);

    // fire_timer: 1→0 → 2回目発射
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    // 1回あたりの発射数が一定であること(circle弾は数が異なるため混入しているとテストNG)
    EXPECT_EQ(enemy_bullets.CountActive(), shot1 * 2u);
}

/**
 * @brief SpecialWarningステップ中は新規弾が発射されないことを確認
 *
 * 契約:
 *   - SpecialWarningステップのswitchケースはbreakのみ(攻撃なし)
 *
 * シーケンス: [{AimedPressure,1}, {SpecialWarning,10}]
 * AimedPressureを1tickで終わらせてSpecialWarningへ遷移し,
 * その後10tick中に弾数が変化しないことを検証する.
 */
TEST(BossAttackSequenceTest, SpecialWarning_EmitsNoNewBullets){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks    = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure,  1, BossShotDef{3, 1, 100.0f, 3.0f, 0.0f}},
        {BossAttackStep::SpecialWarning, 10},
    };
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting (1 tick)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // AimedPressure: 1tickで消費
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 1→0

    // SpecialWarningへ遷移するtick(remaining==0 → AdvanceAttackStep)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    ASSERT_EQ(bs.GetBoss().attack_step, BossAttackStep::SpecialWarning);

    // SpecialWarning中の弾数スナップショット
    const std::size_t count_on_enter = enemy_bullets.CountActive();

    // SpecialWarning中に8tick分進めても弾数は変化しない
    for(int i = 0; i < 8; ++i){
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
        EXPECT_EQ(enemy_bullets.CountActive(), count_on_enter)
            << "SpecialWarning tick " << (i + 1) << " で弾が増加した";
    }
}

/**
 * @brief CircleBarrageステップ中は全方向弾のみ発射されることを確認
 *
 * 契約:
 *   - CircleBarrageステップではUpdateCircleAttack()のみ呼ばれる
 *   - 1発目と2発目で増加する弾数が等しい
 *
 * シーケンス: [{AimedPressure,1}, {CircleBarrage,999}]
 * AimedPressureを速やかに通過してCircleBarrageへ遷移する.
 */
TEST(BossAttackSequenceTest, CircleBarrage_EmitsCircleOnly){
    BossSystem bs;
    BulletSystem enemy_bullets(200);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks    = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure,  1,   BossShotDef{3, 999, 100.0f, 3.0f, 0.5f}},
        {BossAttackStep::CircleBarrage, 999,  BossShotDef{12, 2, 150.0f, 3.0f, 0.0f}},
    };
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // AimedPressure: 1tickで消費(fire_interval_tick=999なので発射なし)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 1→0

    // CircleBarrageへ遷移するtick: fire_timer2→1(発射なし)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    ASSERT_EQ(bs.GetBoss().attack_step, BossAttackStep::CircleBarrage);
    EXPECT_EQ(enemy_bullets.CountActive(), 0u); // timer=1, ここではまだ発射されていない

    // fire_timer: 1→0 → 1回目発射
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    const std::size_t shot1 = enemy_bullets.CountActive();  // 全包囲弾で発射された弾を比較ように記録
    EXPECT_GT(shot1, 0u);

    // fire_timer: 2→1 (増えない)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(enemy_bullets.CountActive(), shot1);

    // fire_timer: 1→0 → 2回目発射
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    // 1回あたりの発射数が一定であること(aimed弾は数が異なるため混入すれば崩れる)
    EXPECT_EQ(enemy_bullets.CountActive(), shot1 * 2u);
}

/**
 * @brief Recoveryステップ中は新規弾が発射されないことを確認
 *
 * 契約:
 *   - Recoveryステップのswitchケースはbreakのみ(攻撃なし)
 *
 * シーケンス: [{AimedPressure,1}, {Recovery,10}]
 */
TEST(BossAttackSequenceTest, Recovery_EmitsNoNewBullets){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks    = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure, 1, BossShotDef{3, 1, 100.0f, 3.0f, 0.0f}},
        {BossAttackStep::Recovery,      10},
    };
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // AimedPressure: 1tickで消費
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 1→0

    // Recoveryへ遷移するtick
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    ASSERT_EQ(bs.GetBoss().attack_step, BossAttackStep::Recovery);

    // Recovery中の弾数スナップショット
    const std::size_t count_on_enter = enemy_bullets.CountActive();

    // Recovery中に8tick進めても弾数が変化しない
    for(int i = 0; i < 8; ++i){
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
        EXPECT_EQ(enemy_bullets.CountActive(), count_on_enter)
            << "Recovery tick " << (i + 1) << " で弾が増加した";
    }
}

/**
 * @brief Reset後にStart+同一シーケンスを実行すると同一の弾出力が再現されることを確認
 *
 * 契約:
 *   - BossSystemは決定論的であり，同一configで再実行すると
 *     各tickのattack_stepと発射弾数が完全に一致する
 *
 * 手順:
 *   1. Run A: K tick 分の (attack_step, bullet_count)を記録
 *   2. Reset → Start(同一 onfig) → Run B: 同じKtick分を記録
 *   3. 両者が完全一致することを検証
 */
TEST(BossAttackSequenceTest, AttackSequence_ReplayDeterministic){
    const int K = 20;  // 検証するtick数(ステップ遷移を複数含む長さ)
    // Boss設定
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks    = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure, 3, BossShotDef{3, 2, 100.0f, 3.0f, 0.5f}},
        {BossAttackStep::CircleBarrage, 4, BossShotDef{12, 2, 150.0f, 3.0f, 0.0f}},
    };

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // TickごとのBossAttackStep/弾数を記録する用構造体
    struct TickSnapshot{
        BossAttackStep step;
        std::size_t    bullet_count;
    };

    // Run A
    BossSystem bs_a;
    BulletSystem bullets_a(500);
    PatternSystem patterns_a;
    bs_a.Start(cfg);
    // Run Aを走らせる
    std::vector<TickSnapshot> run_a;
    run_a.reserve(K);
    for(int i = 0; i < K; ++i){
        bs_a.UpdateTick(dt, player_pos, GetTestRng(), patterns_a, bullets_a);
        run_a.push_back({bs_a.GetBoss().attack_step, bullets_a.CountActive()});
    }

    // Run B: Run Aと同一configを再構築してReset → Start
    BossConfig cfg2 = MakeDefaultConfig();
    cfg2.entering_duration_ticks    = 1;
    cfg2.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure, 3, BossShotDef{3, 2, 100.0f, 3.0f, 0.5f}},
        {BossAttackStep::CircleBarrage, 4, BossShotDef{12, 2, 150.0f, 3.0f, 0.0f}},
    };

    BossSystem bs_b;
    BulletSystem bullets_b(500);
    PatternSystem patterns_b;
    bs_b.Start(cfg2);
    // Run Bを走らせる
    std::vector<TickSnapshot> run_b;
    run_b.reserve(K);
    for(int i = 0; i < K; ++i){
        bs_b.UpdateTick(dt, player_pos, GetTestRng(), patterns_b, bullets_b);
        run_b.push_back({bs_b.GetBoss().attack_step, bullets_b.CountActive()});
    }

    // Run AとRun Bで比較 
    ASSERT_EQ(run_a.size(), run_b.size());
    for(int i = 0; i < K; ++i){
        EXPECT_EQ(run_a[i].step, run_b[i].step)
            << "tick " << i << " で attack_step が不一致";
        EXPECT_EQ(run_a[i].bullet_count, run_b[i].bullet_count)
            << "tick " << i << " で bullet_count が不一致";
    }
}

/**
 * @brief シーケンスの最終ステップを超えるとindex0にwrap-aroundすることを確認
 *
 * 契約:
 *   - AdvanceAttackStepは(index + 1) % sequence.size()で循環する
 *   - wrap-around後はattack_sequence[0].duration_ticksが使用される
 *
 * 3ステップ: [{AimedPressure,2}, {SpecialWarning,2}, {CircleBarrage,2}]
 * AimedPressure(2) → SpecialWarning(2) → CircleBarrage(2) → AimedPressure(2)に戻る
 */
TEST(BossAttackSequenceTest, AttackSequence_LoopsAroundAfterLastStep){
    BossSystem bs;
    BulletSystem enemy_bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks    = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure,  2, BossShotDef{3, 999, 100.0f, 3.0f, 0.0f}},
        {BossAttackStep::SpecialWarning, 2},
        {BossAttackStep::CircleBarrage,  2, BossShotDef{3, 999, 100.0f, 3.0f, 0.0f}},
    };
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // AimedPressure: 2tick(remaining 2→1→0)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::AimedPressure);

    // SpecialWarningへ遷移
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::SpecialWarning);

    // SpecialWarning: 2tick継続(remaining 2→1→0)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::SpecialWarning);

    // CircleBarrageへ遷移
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::CircleBarrage);

    // CircleBarrage: 2tick継続 — 遷移tick(remaining 2→1)に続き残り1tick(1→0)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets); // remaining 1→0
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::CircleBarrage);

    // wrap-around: remaining==0 → AdvanceAttackStep → AimedPressure(index 0)へ遷移
    // EnterAttackStep(0): remaining=2 → --remaining → 1
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, enemy_bullets);
    EXPECT_EQ(bs.GetBoss().attack_step, BossAttackStep::AimedPressure);
    EXPECT_EQ(bs.GetBoss().attack_step_ticks_remaining, 1u); // 遷移tick内で1回デクリメント済み
}

/**
 * @brief AdvanceAttackStep経由でEnterAttackStepが呼ばれると
 *        aimed_fire_timerとcircle_fire_timerが各interval値にリセットされることを確認
 *
 * 契約: ステップ遷移のたびに両タイマーがリセットされるため,
 *       各ステップの先頭では必ずinterval tick後に初回発射が起きる
 *
 * 検証方法:
 *   AimedPressure(duration=2, fire_interval_tick=3)でタイマーを途中まで消費し,
 *   SpecialWarning(duration=1)を挟んで再度AimedPressureに入る
 *   リセットなし: タイマー残1 → 遷移tick内で即発射
 *   リセットあり: タイマーがfire_interval_tick(3)に戻る → 遷移から3tick後に初回発射
 */
TEST(BossAttackSequenceTest, AttackStep_ResetsFireTimerOnEntry){
    BossSystem bs;
    BulletSystem bullets(200);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure,  2,   BossShotDef{3, 3, 100.0f, 3.0f, 0.5f}},  // step 0
        {BossAttackStep::SpecialWarning, 1},    // step 1: 1tickでAimedPressure
        {BossAttackStep::AimedPressure,  999, BossShotDef{3, 3, 100.0f, 3.0f, 0.5f}},  // step 2
    };
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // AimedPressure step 0: 2tick(interval=3なので発射なし, timer: 3→2→1)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets); // timer 3→2
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets); // timer 2→1
    EXPECT_EQ(bullets.CountActive(), 0u);

    // SpecialWarning へ遷移: EnterAttackStep → fire_timer=0(shot定義なし)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().attack_step, BossAttackStep::SpecialWarning);

    // AimedPressure step2 へ遷移: EnterAttackStep → fire_timer=fire_interval_tick(3)にリセット
    // 遷移tick内で switch: AimedPressure → timer 3→2(発射なし)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().attack_step, BossAttackStep::AimedPressure);
    EXPECT_EQ(bullets.CountActive(), 0u); // リセット直後なのでまだ発射なし

    // タイマー 2→1(まだ発射なし)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    EXPECT_EQ(bullets.CountActive(), 0u);

    // タイマー 1→0 → 発射: リセットされていれば3tick後に初めてfire
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    EXPECT_GT(bullets.CountActive(), 0u);
}

/**
 * @brief 最終ステップのduration_ticksを使い切った次のtickで正確にindex 0
 *        wrap-aroundすることを確認
 *
 * 契約: AdvanceAttackStepは(index + 1) % sequence.size()で循環し,
 *       最終ステップのremainingが0になった次のtickで遷移が発生する
 *       境界の前後1tickを検証する
 *
 * シーケンス: [{AimedPressure,3}, {CircleBarrage,2}]
 *
 * 累積 fight tick:
 *   1-3: AimedPressure  (remaining 3→2→1→0)
 *   4-5: CircleBarrage  (遷移tick remaining 2→1, 次tick 1→0)
 *   6:   wrap-around → AimedPressure (index=0)
 */
TEST(BossAttackSequenceTest, AttackSequence_LoopsAtExactTick){
    BossSystem bs;
    BulletSystem bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure, 3, BossShotDef{3, 999, 100.0f, 3.0f, 0.0f}},
        {BossAttackStep::CircleBarrage, 2, BossShotDef{3, 999, 100.0f, 3.0f, 0.0f}},
    };
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // fight tick 1-5: 最終ステップ最後のtickまで進める
    // (AimedPressure 3 tick + CircleBarrage 2 tick)
    for(int i = 0; i < 5; ++i){
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    }
    // 境界直前: CircleBarrage のまま (remaining=0 だが step は変わっていない)
    EXPECT_EQ(bs.GetBoss().attack_step,       BossAttackStep::CircleBarrage);
    EXPECT_EQ(bs.GetBoss().attack_step_index, 1u); // まだ最終ステップ

    // 境界 tick (fight tick 6): remaining==0 → wrap-around → AimedPressure(index=0)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    EXPECT_EQ(bs.GetBoss().attack_step,       BossAttackStep::AimedPressure);
    EXPECT_EQ(bs.GetBoss().attack_step_index, 0u); // index が 0 に戻る
}

// ──────────────────────────────────────────────────────
// BossPhaseTest
// ──────────────────────────────────────────────────────
/**
 * @brief 全フェーズ撃破後にphase_indexがphases配列の有効な添字のままであることを確認
 *
 * 契約: TakeDamage()のDestroyed分岐ではphase_indexを加算しない
 *       最終フェーズ(index=1)を撃破してもphase_indexは1(有効な添字)を保つ
 */
TEST(BossPhaseTest, FinalPhase_DestroyedKeepsValidIndex){
    BossSystem bs;
    BulletSystem bullets(100);
    PatternSystem patterns;

    BossConfig cfg = MakeDefaultConfig();
    // BossStatusのEnteringは即終了
    cfg.entering_duration_ticks = 1;
    cfg.phases = {
        { BossPhaseKind::Normal, 5,  0, {{BossAttackStep::AimedPressure, 999, BossShotDef{3, 5, 100.0f, 3.0f, 0.0f}}} },
        { BossPhaseKind::Spell,  10, 0, {{BossAttackStep::AimedPressure, 999, BossShotDef{3, 5, 100.0f, 3.0f, 0.0f}}} },
    };
    bs.Start(cfg);

    const Vec2  player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);
    ASSERT_EQ(bs.GetBoss().phase_index, 0u);

    // Phase0: HP5に対して5ダメージ → PhaseComplete
    EXPECT_EQ(bs.TakeDamage(5), BossDamageResult::PhaseComplete);

    // UpdateTickでphase_transition_pendingを消費 → Phase1へ遷移
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().phase_index, 1u);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // Phase1: 10ダメージ → Destroyed
    EXPECT_EQ(bs.TakeDamage(10), BossDamageResult::Destroyed);

    // phase_indexは最終フェーズの添字(1)のままであり，配列範囲内である
    EXPECT_EQ(bs.GetBoss().phase_index, 1u);
    EXPECT_LT(bs.GetBoss().phase_index, cfg.phases.size());
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Defeating);
}

/**
 * @brief 全フェーズ撃破・Reset後にStart()するとphase_index=0，HP=初期値に戻ること
 *
 * 契約: Reset()でboss_状態を初期化し，Start()はphase_index=0，HP=phases[0].max_hpで開始する
 */
TEST(BossPhaseTest, Start_AfterDefeatStartsAtPhaseZero){
    BossSystem bs;
    BulletSystem bullets(100);
    PatternSystem patterns;

    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases = {
        { BossPhaseKind::Normal, 5,  0, {{BossAttackStep::AimedPressure, 999, BossShotDef{3, 5, 100.0f, 3.0f, 0.0f}}} },
        { BossPhaseKind::Spell,  10, 0, {{BossAttackStep::AimedPressure, 999, BossShotDef{3, 5, 100.0f, 3.0f, 0.0f}}} },
    };
    bs.Start(cfg);

    const Vec2  player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting → Phase0撃破 → Phase1へ → Phase1撃破
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    bs.TakeDamage(5);   // PhaseComplete
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);  // Phase1へ
    bs.TakeDamage(10);  // Destroyed → ここでボスは敗北

    ASSERT_EQ(bs.GetBoss().status, BossStatus::Defeating);
    ASSERT_EQ(bs.GetBoss().phase_index, 1u);

    // Reset後にStart→同じconfigを読み込み，phase_index=0, HP=Phase0のmax_hpで始まる
    bs.Reset();
    bs.Start(cfg);

    EXPECT_EQ(bs.GetBoss().phase_index, 0u);
    EXPECT_EQ(bs.GetBoss().hp, cfg.phases[0].max_hp);
    EXPECT_EQ(bs.GetBoss().status, BossStatus::Entering);
}

/**
 * @brief フェーズ遷移中(phase_transition_pending=true)の追加ダメージはInvalidTargetを返すことを確認
 *
 * 契約: TakeDamage()でPhaseCompleteが返ったtick内では
 *       phase_transition_pending=trueとなり，追加ダメージはInvalidTargetになる
 */
TEST(BossPhaseTest, PhaseComplete_StopsAdditionalHitsSameTick){
    BossSystem bs;
    BulletSystem bullets(100);
    PatternSystem patterns;

    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases = {
        { BossPhaseKind::Normal, 5,  0, {{BossAttackStep::AimedPressure, 999, BossShotDef{3, 5, 100.0f, 3.0f, 0.0f}}} },
        { BossPhaseKind::Spell,  10, 0, {{BossAttackStep::AimedPressure, 999, BossShotDef{3, 5, 100.0f, 3.0f, 0.0f}}} },
    };
    bs.Start(cfg);

    const Vec2  player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;
    // 戦闘開始
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // Phase0: 5ダメージ → PhaseComplete, phase_transition_pending=true
    EXPECT_EQ(bs.TakeDamage(5), BossDamageResult::PhaseComplete);
    EXPECT_TRUE(bs.GetBoss().phase_transition_pending);

    // 同tick内での追加ダメージはInvalidTarget
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::InvalidTarget);
    EXPECT_EQ(bs.TakeDamage(1), BossDamageResult::InvalidTarget);
}

// ──────────────────────────────────────────────────────
// BossShotDefTest
// ──────────────────────────────────────────────────────
/**
 * @brief BossShotDefのbullet_countとbullet_speedが実際の発射弾数・速度に反映されることを確認
 *
 * bullet_count=5, bullet_speed=200.0fで発射すると
 * 5発の弾が生成され各弾の速度ベクトルの大きさが200.0fになる
 */
TEST(BossShotDefTest, ShotDef_ControlsBulletCountAndSpeed){
    BossSystem bs;
    BulletSystem bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    // bullet_count=5, fire_interval_tick=2, bullet_speed=200
    cfg.phases[0].attack_sequence = {{
        BossAttackStep::AimedPressure, 999,
        BossShotDef{5, 2, 200.0f, 3.0f, 0.0f}
    }};
    bs.Start(cfg);

    const Vec2  player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting(fire_timer = 2)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);
    EXPECT_EQ(bullets.CountActive(), 0u);

    // Fight tick 1: timer 2→1(未発射)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    EXPECT_EQ(bullets.CountActive(), 0u);

    // Fight tick 2: timer 1→0 → 5発発射
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bullets.CountActive(), 5u);

    // 各弾の速度ベクトルの大きさがbullet_speed(200.0f)であることを確認
    for(const auto& b : bullets.GetBullets()){
        if(!b.active){
            continue;
        }
        // 進行方向zに対して，x, y軸の速度で平方根を求める(VelocityFromAngle()の逆)
        const float speed = std::hypot(b.velocity.x, b.velocity.y);
        EXPECT_NEAR(speed, 200.0f, 0.01f);
    }
}

/**
 * @brief BossShotDefのfire_interval_tickが発射タイミングに反映されることを確認
 *
 * fire_interval_tick=4の場合，Fighting遷移直後の発射タイマーが4になり，
 * 3tick目までは発射なし・4tick目に初めて発射される
 */
TEST(BossShotDefTest, ShotDef_ControlsFireInterval){
    BossSystem bs;
    BulletSystem bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks   = 1;
    cfg.phases[0].attack_sequence = {{
        BossAttackStep::AimedPressure, 999,
        BossShotDef{3, 4, 100.0f, 3.0f, 0.0f}  // fire_interval_tick=4
    }};
    bs.Start(cfg);

    const Vec2  player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting(fire_timer = 4)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // Fight tick 1-3: 発射なし(timer 4→...→1)
    for(int i = 0; i < 3; ++i){
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    }
    EXPECT_EQ(bullets.CountActive(), 0u);

    // Fight tick 4: timer 1→0 → 発射
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    EXPECT_GT(bullets.CountActive(), 0u);
}

// ──────────────────────────────────────────────────────
// BossMovementTest
// ──────────────────────────────────────────────────────
/**
 * @brief MoveTo/Fixed移動でduration_ticks後に正確に目的座標に到達することを確認
 *
 * move_duration_ticks=5, target={400.0f, 150.0f} の場合，
 * - fight tick 4まで: 未到達 (lerp t < 1.0)
 * - fight tick 5(=duration_ticks): lerp t=1.0 → 正確に目的座標と一致する
 */
TEST(BossMovementTest, MoveTo_ReachesTargetAtExactTick){
    // ボス構成
    BossSystem bs;
    BulletSystem bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].attack_sequence = {{
        BossAttackStep::AimedPressure, 999,
        BossShotDef{3, 999, 100.0f, 3.0f, 0.0f},    // 弾は撃たない(fire_interval_tick=999)
        BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::Fixed, {400.0f, 150.0f}, {}, 5}
    }};
    bs.Start(cfg);

    const Vec2  player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting(EnterAttackStep: move_start={320,80}, move_target={400,150}, duration=5)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // fight tick 1-4: 未到達
    for(int i = 0; i < 4; ++i){
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    }
    EXPECT_NE(bs.GetBoss().position.x, 400.0f);

    // fight tick 5: lerp t=1.0 → 目的座標に到達
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    EXPECT_FLOAT_EQ(bs.GetBoss().position.x, 400.0f);
    EXPECT_FLOAT_EQ(bs.GetBoss().position.y, 150.0f);
}

/**
 * @brief BossMoveType::Stayで位置が変化しないことを確認
 *
 * Stay移動はCalculateMovement内でmove_duration_ticks=0に設定するため，
 * UpdateTick内の移動lerp処理が一切実行されない
 */
TEST(BossMovementTest, Stay_StopsMovement){
    BossSystem bs;
    BulletSystem bullets(100);
    PatternSystem patterns;
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].attack_sequence = {{
        BossAttackStep::AimedPressure, 999,
        BossShotDef{3, 999, 100.0f, 3.0f, 0.0f},
        BossMovementDef{BossMoveType::Stay, BossMoveTarget::Fixed, {320.0f, 100.0f}, {}, 1}
    }};
    bs.Start(cfg);

    const Vec2  player_pos{320.0f, 400.0f};
    const float dt = 1.0f / 60.0f;

    // Entering → Fighting (CalculateMovement: Stay → move_duration_ticks=0)
    bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);
    const Vec2 initial_pos = bs.GetBoss().position;

    // 10fight tick: 位置変化なし
    for(int i = 0; i < 10; ++i){
        bs.UpdateTick(dt, player_pos, GetTestRng(), patterns, bullets);
    }
    EXPECT_FLOAT_EQ(bs.GetBoss().position.x, initial_pos.x);
    EXPECT_FLOAT_EQ(bs.GetBoss().position.y, initial_pos.y);
}

/**
 * @brief OppositePlayerモードでEnterAttackStep時のplayer_positionが移動先計算に使われることを確認
 *
 * ボス2体，プレイヤー2体で検証
 * player1={100,400}: target.x = clamp(640-100, 64, 576) = 540
 * player2={500,400}: target.x = clamp(640-500, 64, 576) = 140
 * どちらも target.y = PLAYFIELD_HEIGHT * 0.25 = 180
 */
TEST(BossMovementTest, FirstMove_UsesActualPlayerPosition){
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].attack_sequence = {{
        BossAttackStep::AimedPressure, 999,
        BossShotDef{3, 999, 100.0f, 3.0f, 0.0f},
        BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::OppositePlayer, {}, {}, 60}
    }};

    BossSystem bs1, bs2;
    BulletSystem bullets(100);
    PatternSystem patterns;
    bs1.Start(cfg);
    bs2.Start(cfg);

    const float dt  = 1.0f / 60.0f;
    const Vec2 player1{100.0f, 400.0f};
    const Vec2 player2{500.0f, 400.0f};

    // 異なるplayer_posでEntering → Fighting → EnterAttackStep(OppositePlayer)
    bs1.UpdateTick(dt, player1, GetTestRng(), patterns, bullets);
    bs2.UpdateTick(dt, player2, GetTestRng(), patterns, bullets);

    ASSERT_EQ(bs1.GetBoss().status, BossStatus::Fighting);
    ASSERT_EQ(bs2.GetBoss().status, BossStatus::Fighting);

    // player1.x=100 → 640-100=540, player2.x=500 → 640-500=140
    EXPECT_FLOAT_EQ(bs1.GetBoss().move_target.x, 540.0f);
    EXPECT_FLOAT_EQ(bs2.GetBoss().move_target.x, 140.0f);
    // Y軸は共通 (PLAYFIELD_HEIGHT * 0.25 = 720 * 0.25 = 180)
    EXPECT_FLOAT_EQ(bs1.GetBoss().move_target.y, 180.0f);
    EXPECT_FLOAT_EQ(bs2.GetBoss().move_target.y, 180.0f);
}

/**
 * @brief 同一シードのDeterministicRngを用いると同一のRandom移動先が得られることを確認
 *
 * 再現性の保証: seed=42のRNG2個で独立したBossSystemを動かすと
 * move_targetが完全に一致する
 */
/**
 * @brief 同一seed・同一configでReset後に再実行した結果が完全一致することを確認
 *
 * シナリオ:
 *   Start(config)
 *   → 同一seed/入力でN tick進める
 *   → Boss位置・フェーズ・攻撃状態・弾の状態を記録
 *   → Reset()
 *   → RNGを同一seedに戻す
 *   → Start(config)
 *   → 同一入力でN tick進める
 *   → スナップショットを比較
 *
 * 比較するフィールド:
 *   - position / phase_index / attack_step_index
 *   - move_target / fire_timer / move_elapsed_ticks
 *   - アクティブ弾数・弾の座標/速度
 *   - RNG消費回数(GetDrawCount()で直接確認)
 */
TEST(BossMovementTest, DeterministicReplay_SameSeedIdenticalState){
    // ── config ──────────────────────────────────────
    // Random移動 + 発射ありの2ステップ構成
    // step[0]: 5tick / fire_interval=2 / Random移動(RNG消費あり)
    // step[1]: 3tick / 発射なし / 移動なし
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure, 5,
         BossShotDef{3, 2, 100.0f, 3.0f, 0.0f},
         BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::Random, {}, {80.0f, 50.0f}, 4}},
        {BossAttackStep::Recovery, 3,
         std::nullopt, std::nullopt},
    };

    constexpr int     N          = 20;  // ステップ遷移を複数含む長さ
    const Vec2        player_pos{320.0f, 400.0f};
    const float       dt         = 1.0f / 60.0f;
    constexpr DeterministicRng::Seed SEED = 42;

    // ── Run A ────────────────────────────────────────
    BossSystem       bs;
    BulletSystem     bullets_a(500);
    PatternSystem    patterns_a;
    DeterministicRng rng_a{SEED};
    bs.Start(cfg);
    for(int i = 0; i < N; ++i){
        bs.UpdateTick(dt, player_pos, rng_a, patterns_a, bullets_a);
    }

    // Run A のスナップショットを記録
    struct BossSnapshot{
        Vec2          position;
        std::size_t   phase_index;
        std::size_t   attack_step_index;
        Vec2          move_target;
        std::uint32_t fire_timer;
        std::uint32_t move_elapsed_ticks;
    };
    struct BulletEntry{
        Vec2 position;
        Vec2 velocity;
    };
    const BossSnapshot snap_a{
        .position          = bs.GetBoss().position,
        .phase_index       = bs.GetBoss().phase_index,
        .attack_step_index = bs.GetBoss().attack_step_index,
        .move_target       = bs.GetBoss().move_target,
        .fire_timer        = bs.GetBoss().fire_timer,
        .move_elapsed_ticks = bs.GetBoss().move_elapsed_ticks,
    };
    std::vector<BulletEntry> bullets_snap_a;
    for(const auto& b : bullets_a.GetBullets()){
        if(b.active){ bullets_snap_a.push_back({b.position, b.velocity}); }
    }
    const std::uint64_t draw_count_a = rng_a.GetDrawCount();

    // ── Reset → Run B (同一seed) ─────────────────────
    bs.Reset();
    bs.Start(cfg);
    BulletSystem     bullets_b(500);
    PatternSystem    patterns_b;
    DeterministicRng rng_b{SEED};
    for(int i = 0; i < N; ++i){
        bs.UpdateTick(dt, player_pos, rng_b, patterns_b, bullets_b);
    }

    // ── 比較 ─────────────────────────────────────────
    const auto& boss_b = bs.GetBoss();

    EXPECT_FLOAT_EQ(boss_b.position.x,       snap_a.position.x);
    EXPECT_FLOAT_EQ(boss_b.position.y,       snap_a.position.y);
    EXPECT_EQ(      boss_b.phase_index,       snap_a.phase_index);
    EXPECT_EQ(      boss_b.attack_step_index, snap_a.attack_step_index);
    EXPECT_FLOAT_EQ(boss_b.move_target.x,    snap_a.move_target.x);
    EXPECT_FLOAT_EQ(boss_b.move_target.y,    snap_a.move_target.y);
    EXPECT_EQ(      boss_b.fire_timer,        snap_a.fire_timer);
    EXPECT_EQ(      boss_b.move_elapsed_ticks, snap_a.move_elapsed_ticks);

    // 弾の状態比較
    std::vector<BulletEntry> bullets_snap_b;
    for(const auto& b : bullets_b.GetBullets()){
        if(b.active){ bullets_snap_b.push_back({b.position, b.velocity}); }
    }
    ASSERT_EQ(bullets_snap_b.size(), bullets_snap_a.size());
    for(std::size_t i = 0; i < bullets_snap_a.size(); ++i){
        EXPECT_FLOAT_EQ(bullets_snap_b[i].position.x, bullets_snap_a[i].position.x)
            << "bullet[" << i << "].position.x が不一致";
        EXPECT_FLOAT_EQ(bullets_snap_b[i].position.y, bullets_snap_a[i].position.y)
            << "bullet[" << i << "].position.y が不一致";
        EXPECT_FLOAT_EQ(bullets_snap_b[i].velocity.x, bullets_snap_a[i].velocity.x)
            << "bullet[" << i << "].velocity.x が不一致";
        EXPECT_FLOAT_EQ(bullets_snap_b[i].velocity.y, bullets_snap_a[i].velocity.y)
            << "bullet[" << i << "].velocity.y が不一致";
    }

    // RNG消費回数が同一であることを確認
    EXPECT_EQ(rng_b.GetDrawCount(), draw_count_a);
}

/**
 * @brief MoveTo 完了後，次のステップで movement = nullopt ならば
 *        move_type == Stay になり，位置と move_elapsed_ticks が変化しないことを確認
 *
 * step[0]: AimedPressure, duration=60, MoveTo Fixed target_pos, move_duration=5
 * step[1]: Recovery,      duration=999, movement=nullopt
 *
 * 動作:
 *   Entering(1) → Fighting step[0](60) → Fighting step[1]
 *   step[1] 開始時に CalculateMovement が nullopt → move_type=Stay, elapsed=0
 *   以降のTickで位置・elapsed が変化しない
 */
TEST(BossMovementTest, NulloptMovement_SetsStayAndStopsMovement){
    constexpr std::uint32_t STEP0_DURATION = 60;
    constexpr std::uint32_t MOVE_DURATION  = 5;
    const Vec2 start_pos {320.0f, 100.0f};
    const Vec2 target_pos{200.0f,  90.0f};

    BossConfig cfg = MakeDefaultConfig();
    cfg.position                = start_pos;
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].attack_sequence = {
        {BossAttackStep::AimedPressure, STEP0_DURATION,
         BossShotDef{3, 999, 100.0f, 3.0f, 0.0f},
         BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::Fixed, target_pos, {}, MOVE_DURATION}},
        {BossAttackStep::Recovery, 999,
         std::nullopt, std::nullopt},
    };

    BossSystem       bs;
    DeterministicRng rng{0};
    BulletSystem     bullets(10);
    PatternSystem    patterns;
    bs.Start(cfg);

    const float dt         = 1.0f / 60.0f;
    const Vec2  player_pos{320.0f, 400.0f};

    // Entering を抜ける → step[0] 開始
    bs.UpdateTick(dt, player_pos, rng, patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);
    EXPECT_EQ(bs.GetBoss().move_type, BossMoveType::MoveTo);

    // step[0] を消化して step[1] へ遷移させる
    // UpdateFighting は remaining==0 を次tick冒頭で検出するため +1 tick 必要
    for(std::uint32_t i = 0; i < STEP0_DURATION + 1; ++i){
        bs.UpdateTick(dt, player_pos, rng, patterns, bullets);
    }

    // step[1] に遷移済み
    ASSERT_EQ(bs.GetBoss().attack_step_index, 1u);

    // move_type が Stay にリセットされている
    EXPECT_EQ(bs.GetBoss().move_type, BossMoveType::Stay);

    // 座標は step[0] の MoveTo 完了後のまま (target_pos に到達済み)
    EXPECT_FLOAT_EQ(bs.GetBoss().position.x, target_pos.x);
    EXPECT_FLOAT_EQ(bs.GetBoss().position.y, target_pos.y);

    // さらに数ティック進めても座標・elapsed が変化しない
    const Vec2          pos_before     = bs.GetBoss().position;
    const std::uint32_t elapsed_before = bs.GetBoss().move_elapsed_ticks;
    for(int i = 0; i < 3; ++i){
        bs.UpdateTick(dt, player_pos, rng, patterns, bullets);
    }
    EXPECT_FLOAT_EQ(bs.GetBoss().position.x, pos_before.x);
    EXPECT_FLOAT_EQ(bs.GetBoss().position.y, pos_before.y);
    EXPECT_EQ(bs.GetBoss().move_elapsed_ticks, elapsed_before);
}

// ──────────────────────────────────────────────────────
// BossConfigValidationTest
// ──────────────────────────────────────────────────────
/**
 * @brief BossShotDef/BossMovementDefの不正値でStart()がinvalid_argumentをthrowすることを確認
 *
 * 検証する無効ケース:
 *   - BossShotDef: fire_interval_tick = 0
 *   - BossShotDef: bullet_count = 0
 *   - BossShotDef: bullet_speed = 0
 *   - BossMovementDef Fixed: target_position.x = 0
 *   - BossMovementDef Random: move_range.x = 0
 *   - BossMovementDef: duration_ticks = 0
 */
TEST(BossConfigValidationTest, Config_RejectsInvalidShotAndMovement){
    // fire_interval_tick = 0
    {
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{
            BossAttackStep::AimedPressure, 10,
            BossShotDef{3, 0, 100.0f, 3.0f, 0.0f}
        }};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    // bullet_count = 0
    {
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{
            BossAttackStep::AimedPressure, 10,
            BossShotDef{0, 5, 100.0f, 3.0f, 0.0f}
        }};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    // bullet_speed = 0
    {
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{
            BossAttackStep::AimedPressure, 10,
            BossShotDef{3, 5, 0.0f, 3.0f, 0.0f}
        }};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    // Fixed: target_position.x = 0
    {
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{
            BossAttackStep::AimedPressure, 10,
            BossShotDef{3, 5, 100.0f, 3.0f, 0.0f},
            BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::Fixed, {0.0f, 100.0f}, {}, 10}
        }};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    // Random: move_range.x = 0
    {
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{
            BossAttackStep::AimedPressure, 10,
            BossShotDef{3, 5, 100.0f, 3.0f, 0.0f},
            BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::Random, {}, {0.0f, 50.0f}, 10}
        }};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
    // duration_ticks = 0
    {
        BossSystem bs;
        BossConfig cfg = MakeDefaultConfig();
        cfg.phases[0].attack_sequence = {{
            BossAttackStep::AimedPressure, 10,
            BossShotDef{3, 5, 100.0f, 3.0f, 0.0f},
            BossMovementDef{BossMoveType::MoveTo, BossMoveTarget::Fixed, {320.0f, 100.0f}, {}, 0}
        }};
        EXPECT_THROW(bs.Start(cfg), std::invalid_argument);
    }
}

/**
 * @brief BossMovementDef{}(デフォルト==Stay)がValidationで拒否されないこと
 *
 * Stayタイプではtarget_position/move_range/duration_ticksは未使用のためValidationがスキップされる
 * BossMovementDefのデフォルト値(全ゼロ)が誤って拒否されないことを回帰テストで確かめる
 */
TEST(BossConfigValidationTest, Config_AcceptsDefaultMovementDef){
    BossSystem bs;
    BossConfig cfg = MakeDefaultConfig();
    cfg.phases[0].attack_sequence = {{
        BossAttackStep::AimedPressure, 10,
        BossShotDef{3, 5, 100.0f, 3.0f, 0.0f},
        BossMovementDef{}   // Stay, duration=0, target={0,0} — Stayなので検証スキップ
    }};
    EXPECT_NO_THROW(bs.Start(cfg));
}

// ──────────────────────────────────────────────────────
// BossTeleportationTest
// ──────────────────────────────────────────────────────
/**
 * @brief CalculateMovement()でboss.move_type=Teleportationにセットされるか
 *
 * Entering → Fighting遷移直後にboss.move_type==Teleportationとなる
 */
TEST(BossTeleportationTest, Teleportation_SetsMoveType){
    BossConfig cfg = MakeDefaultConfig();
    cfg.entering_duration_ticks = 1;
    // Teleportationを設定
    cfg.phases[0].attack_sequence = {{
        BossAttackStep::AimedPressure, 999,
        BossShotDef{3, 999, 100.0f, 3.0f, 0.0f},
        BossMovementDef{BossMoveType::Teleportation, BossMoveTarget::Fixed, {320.0f, 100.0f}, {}, 30}
    }};

    BossSystem bs;
    DeterministicRng rng{0};
    BulletSystem bullets(10);
    PatternSystem patterns;
    bs.Start(cfg);

    const Vec2 player_pos{320.0f, 400.0f};
    bs.UpdateTick(1.0f / 60.0f, player_pos, rng, patterns, bullets);

    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);
    EXPECT_EQ(bs.GetBoss().move_type, BossMoveType::Teleportation);
}

/**
 * @brief duration - 1tick経過後もpositionが変わらないことを確認
 *
 * Teleportationはカウントダウン中に座標を変えない
 */
TEST(BossTeleportationTest, Teleportation_DoesNotMoveBeforeDeadline){
    constexpr std::uint32_t DURATION = 10;
    const Vec2 start_pos {320.0f, 100.0f};
    const Vec2 target_pos{100.0f,  80.0f};

    BossConfig cfg = MakeDefaultConfig();
    cfg.position                  = start_pos;
    cfg.entering_duration_ticks   = 1;
    cfg.phases[0].attack_sequence = {{
        BossAttackStep::AimedPressure, 999,
        BossShotDef{3, 999, 100.0f, 3.0f, 0.0f},
        BossMovementDef{BossMoveType::Teleportation, BossMoveTarget::Fixed, target_pos, {}, DURATION}
    }};

    BossSystem bs;
    DeterministicRng rng{0};
    BulletSystem bullets(10);
    PatternSystem patterns;
    bs.Start(cfg);

    const float dt = 1.0f / 60.0f;
    const Vec2  player_pos{320.0f, 400.0f};

    // Enteringを抜ける
    bs.UpdateTick(dt, player_pos, rng, patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // duration - 1 tick進める(スナップの1tick手前)
    for(std::uint32_t i = 0; i < DURATION - 1; ++i){
        bs.UpdateTick(dt, player_pos, rng, patterns, bullets);
    }

    // まだ元の位置のまま
    EXPECT_FLOAT_EQ(bs.GetBoss().position.x, start_pos.x);
    EXPECT_FLOAT_EQ(bs.GetBoss().position.y, start_pos.y);
}

/**
 * @brief duration_tick到達後にtarget_positionへスナップする
 *
 * move_elapsed_ticks == move_duration_ticksになったtickで
 * positionが一瞬でtarget_positionに移動することを確認
 */
TEST(BossTeleportationTest, Teleportation_SnapsToPosAtExactTick){
    constexpr std::uint32_t DURATION = 10;
    const Vec2 start_pos {320.0f, 100.0f};
    const Vec2 target_pos{100.0f,  80.0f};

    BossConfig cfg = MakeDefaultConfig();
    cfg.position                = start_pos;
    cfg.entering_duration_ticks = 1;
    cfg.phases[0].attack_sequence = {{
        BossAttackStep::AimedPressure, 999,
        BossShotDef{3, 999, 100.0f, 3.0f, 0.0f},
        BossMovementDef{BossMoveType::Teleportation, BossMoveTarget::Fixed, target_pos, {}, DURATION}
    }};

    BossSystem bs;
    DeterministicRng rng{0};
    BulletSystem bullets(10);
    PatternSystem patterns;
    bs.Start(cfg);

    const float dt         = 1.0f / 60.0f;
    const Vec2  player_pos{320.0f, 400.0f};

    // Entering を抜ける
    bs.UpdateTick(dt, player_pos, rng, patterns, bullets);
    ASSERT_EQ(bs.GetBoss().status, BossStatus::Fighting);

    // duration_tickを進める(スナップが発生するtickまで)
    for(std::uint32_t i = 0; i < DURATION; ++i){
        bs.UpdateTick(dt, player_pos, rng, patterns, bullets);
    }

    // target_positionへスナップしているか
    EXPECT_FLOAT_EQ(bs.GetBoss().position.x, target_pos.x);
    EXPECT_FLOAT_EQ(bs.GetBoss().position.y, target_pos.y);
}
