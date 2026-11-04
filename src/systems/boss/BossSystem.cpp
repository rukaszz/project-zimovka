#include "zimovka/systems/boss/BossSystem.hpp"

#include <cmath>
#include <numbers>
#include <stdexcept>

#include "zimovka/math/GameplayMath.hpp"
#include "zimovka/systems/pattern/PatternEmitRequest.hpp"

namespace zimovka{
// ヘルパ関数
namespace{
/**
 * @brief 無限大チェック用ヘルパ関数
 *
 * @param value
 * @return true
 * @return false
 */
bool IsFinite(const Vec2& value) noexcept{
    return std::isfinite(value.x) && std::isfinite(value.y);
}

// 自機狙い拡散弾パラメータ(実装確認用の仮の値)
constexpr std::uint32_t AimedBulletCount  = 5;
constexpr float         AimedSpreadRad    = std::numbers::pi_v<float> / 2.0f;  // 90°
constexpr float         AimedBulletSpeed  = 200.0f;
constexpr float         AimedBulletRadius = 3.0f;
// 全方向弾パラメータ(プロトタイプ固定値)
constexpr std::uint32_t CircleBulletCount  = 12;
constexpr float         CircleBulletSpeed  = 150.0f;
constexpr float         CircleBulletRadius = 4.0f;
}   // namespace

/**
 * @brief ボス戦開始処理
 *
 * @param config
 */
void BossSystem::Start(const BossConfig& config){
    // Inactiveな状態で一度のみ呼ばれなければならない
    if(boss_.status!= BossStatus::Inactive){
        throw std::logic_error(
            "BossSystem::Start requires Inactive state. "
        );
    }
    // 引数configのチェック
    if(config.name.empty()){
        throw std::invalid_argument(
            "Invalid Boss config: name is empty. "
        );
    }
    if(!IsFinite(config.position)
    || !IsFinite(config.velocity)
    || !IsFinite(config.render_size)
    || (config.render_size.x <= 0 || config.render_size.y <= 0))
    {
        throw std::invalid_argument(
            "Invalid Boss render size or non-finite vector. "
        );
    }
    const bool valid_radius = config.hurtbox_radius > 0.0f
                           && config.contact_radius > 0.0f
                           && std::isfinite(config.hurtbox_radius)
                           && std::isfinite(config.contact_radius);
    const bool valid_timers = config.entering_duration_ticks > 0
                           && config.attack_step_duration_ticks > 0
                           && config.defeating_duration_ticks > 0
                           && config.aimed_fire_interval > 0
                           && config.circle_fire_interval > 0;
    if(!valid_radius || !valid_timers || config.max_hp <= 0){
        throw std::invalid_argument(
            "Invalid Boss config: invalid integer values."
        );
    }
    // configの適用
    boss_.name        = config.name;

    boss_.position    = config.position;
    boss_.velocity    = config.velocity;
    boss_.render_size = config.render_size;

    boss_.hurtbox_radius = config.hurtbox_radius;
    boss_.contact_radius = config.contact_radius;

    boss_.hp     = config.max_hp;
    boss_.max_hp = config.max_hp;

    boss_.entering_ticks_remaining    = config.entering_duration_ticks;
    boss_.attack_step_ticks_remaining = config.attack_step_duration_ticks;
    boss_.defeating_duration_ticks    = config.defeating_duration_ticks;
    boss_.defeating_ticks_remaining   = config.defeating_duration_ticks;

    boss_.aimed_duration_ticks        = config.aimed_duration_ticks;
    boss_.waiting_duration_ticks      = config.waiting_duration_ticks;
    boss_.circle_duration_ticks       = config.circle_duration_ticks;
    boss_.recovery_duration_ticks     = config.recovery_duration_ticks;

    // タイマー: 最初の発射はinterval経過後に来るよう初期化
    boss_.aimed_fire_interval  = config.aimed_fire_interval;
    boss_.aimed_fire_timer     = config.aimed_fire_interval;
    boss_.circle_fire_interval = config.circle_fire_interval;
    boss_.circle_fire_timer    = config.circle_fire_interval;

    boss_.status = BossStatus::Entering;
    boss_.attack_step = BossAttackStep::AimedPressure;
}

/**
 * @brief ボスのリセット
 *
 */
void BossSystem::Reset() noexcept{
    boss_ = Boss{};
}

/**
 * @brief ボスの更新処理
 *
 * 状態遷移:
 *   Entering → fight_tickカウントダウン → Fighting
 *   Fighting → タイマー管理・弾発射
 *   Defeating → Defeatedイベントを返す
 *
 * @param dt
 * @param player_position
 * @param patterns
 * @param enemy_bullets
 * @return BossEvents
 */
BossTickEvents BossSystem::UpdateTick(
    float dt,
    const Vec2& player_position,
    PatternSystem& patterns,
    BulletSystem& enemy_bullets
)
{
    // 処理しないパターン
    if(boss_.status == BossStatus::Inactive){
        return {};
    }
    // ボスが敗北したとき
    if(boss_.status == BossStatus::Defeating){
        if(boss_.defeating_ticks_remaining > 0){
            --boss_.defeating_ticks_remaining;
        }
        // remaining == 0: 敗北演出が完了
        if(boss_.defeating_ticks_remaining == 0){
            boss_.status = BossStatus::Inactive;
            return BossTickEvents{
                .defeat_animation_completed = true
            };
        }
        return {};
    }
    // 座標更新
    boss_.position += boss_.velocity * dt;

    // ─── Entering ─────────────────────────────
    // Entering: fight_tick消費 → Fightingへ遷移
    if(boss_.status == BossStatus::Entering){
        // tick消費と残0判定で2回ifが必要
        if(boss_.entering_ticks_remaining > 0){
            --boss_.entering_ticks_remaining;
        }
        if(boss_.entering_ticks_remaining == 0){
            boss_.status = BossStatus::Fighting;
        }
        return {};
    }

    // ─── Fighting ─────────────────────────────
    // Fighting: パターン発射
    // 自機狙い拡散弾 (EmitSpread)
    if(boss_.aimed_fire_timer > 0){
        --boss_.aimed_fire_timer;
    }
    if(boss_.aimed_fire_timer == 0){
        UpdateAimedAttack(player_position, patterns, enemy_bullets);
        boss_.aimed_fire_timer = boss_.aimed_fire_interval;
    }

    // 全方向弾 (EmitCircle)
    if(boss_.circle_fire_timer > 0){
        --boss_.circle_fire_timer;
    }
    if(boss_.circle_fire_timer == 0){
        UpdateCircleAttack(patterns, enemy_bullets);
        boss_.circle_fire_timer = boss_.circle_fire_interval;
    }

    return {};
}

/**
 * @brief ボスのダメージ処理(処理結果を返す)
 *
 * @param damage
 * @return BossDamageResult
 */
BossDamageResult BossSystem::TakeDamage(std::int32_t damage){
    // Fightingでのみダメージを受ける
    if(damage <= 0 || boss_.status != BossStatus::Fighting){
        return BossDamageResult::InvalidTarget;
    }
    // hpを超えるダメージで撃破
    if(damage >= boss_.hp){
        boss_.hp = 0;
        boss_.status = BossStatus::Defeating;
        // 敗北演出処理時間セット
        boss_.defeating_ticks_remaining = boss_.defeating_duration_ticks;
        return BossDamageResult::Destroyed;
    }
    // ダメージ処理
    boss_.hp -= damage;
    return BossDamageResult::Damaged;
}

/**
 * @brief ボスの攻撃パターン制御
 * 
 * attack_step_ticksによるBossAttackStepの切り替え
 */
void BossSystem::AdvanceAttackStep(){
    switch(boss_.attack_step){
    case BossAttackStep::AimedPressure:
        boss_.attack_step = BossAttackStep::SpecialWarning;
        boss_.attack_step_ticks_remaining = boss_.waiting_duration_ticks;
        break;
    case BossAttackStep::SpecialWarning:
        boss_.attack_step = BossAttackStep::CircleBarrage;
        boss_.attack_step_ticks_remaining = boss_.circle_duration_ticks;
        break;
    case BossAttackStep::CircleBarrage:
        boss_.attack_step = BossAttackStep::Recovery;
        boss_.attack_step_ticks_remaining = boss_.recovery_duration_ticks;
        break;
    case BossAttackStep::Recovery:
        boss_.attack_step = BossAttackStep::AimedPressure;
        boss_.attack_step_ticks_remaining = boss_.aimed_duration_ticks;
        break;
    default:
        // 不正なenum
        break;
    }
}

/**
 * @brief 自機狙い扇形弾
 * 
 * @param player_position 
 * @param patterns 
 * @param enemy_bullets 
 * @return std::size_t 発射した弾数
 */
std::size_t BossSystem::UpdateAimedAttack(
    const Vec2& player_position, 
    PatternSystem& patterns, 
    BulletSystem& enemy_bullets
)
{
    PatternEmitRequest req{};
    req.origin         = boss_.position;
    req.base_angle_rad = GameplayMath::AimAngleRad(boss_.position, player_position);
    req.bullet_count   = AimedBulletCount;
    req.spread_rad     = AimedSpreadRad;
    req.bullet_speed   = AimedBulletSpeed;
    req.bullet_radius  = AimedBulletRadius;
    return patterns.EmitSpread(req, enemy_bullets);
}

/**
 * @brief 全包囲弾発射
 * 
 * @param player_position 
 * @param patterns 
 * @param enemy_bullets 
 * @return std::size_t 
 */
std::size_t BossSystem::UpdateCircleAttack(
    PatternSystem& patterns, 
    BulletSystem& enemy_bullets
)
{
    PatternEmitRequest req{};
    req.origin         = boss_.position;
    req.base_angle_rad = 0.0f;
    req.bullet_count   = CircleBulletCount;
    req.bullet_speed   = CircleBulletSpeed;
    req.bullet_radius  = CircleBulletRadius;
    return patterns.EmitCircle(req, enemy_bullets);
}

/**
 * @brief 戦闘時のパターン別攻撃制御
 * 
 * BossStatus::Fightingの際に実行される
 * 契約として，残り時間0になった次の更新で状態を移行する
 * 
 * @param player_position 
 * @param patterns 
 * @param enemy_bullets 
 */
void BossSystem::UpdateFighting(
    const Vec2& player_position, 
    PatternSystem& patterns, 
    BulletSystem& enemy_bullets
)
{
    // 攻撃ステップの残時間0で次の攻撃状態へ
    if(boss_.attack_step_ticks_remaining == 0){
        AdvanceAttackStep();
    }
    // BossAttackStepに応じて行動パターンを返る
    switch(boss_.attack_step){
    // 自機狙い弾攻撃
    case BossAttackStep::AimedPressure:
        UpdateAimedAttack(
            player_position, 
            patterns, 
            enemy_bullets
        );
        break;
    // 形態移行中
    case BossAttackStep::SpecialWarning:
        break;  // 攻撃しない
    // 全包囲攻撃
    case BossAttackStep::CircleBarrage:
        UpdateCircleAttack(
            patterns, 
            enemy_bullets
        );
    // フェーズの初期化
    case BossAttackStep::Recovery:
        break;  // 攻撃しない
    default:
        // 不正なenum
        break;
    }
    --boss_.attack_step_ticks_remaining;
}

}   // namespace zimovka
