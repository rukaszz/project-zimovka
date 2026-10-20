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
            "Invalid Boss config: name is empty."
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

    boss_.entering_ticks_remaining = config.entering_duration_ticks;

    // タイマー: 最初の発射はinterval後に来るよう初期化
    boss_.aimed_fire_interval  = config.aimed_fire_interval;
    boss_.aimed_fire_timer     = config.aimed_fire_interval;
    boss_.circle_fire_interval = config.circle_fire_interval;
    boss_.circle_fire_timer    = config.circle_fire_interval;

    boss_.status = BossStatus::Entering;
}

/**
 * @brief ボスのリセット
 *
 * 現状はHPのみ
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
BossEvents BossSystem::UpdateTick(
    float dt,
    const Vec2& player_position,
    PatternSystem& patterns,
    BulletSystem& enemy_bullets
)
{
    // 処理しないパターン
    if(boss_.status == BossStatus::Inactive){
        return BossEvents::None;
    }
    if(boss_.status == BossStatus::Defeating){
        return BossEvents::Defeated;
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
        return BossEvents::WaitingTrigger;
    }

    // ─── Fighting ─────────────────────────────
    // Fighting: パターン発射
    // 自機狙い拡散弾 (EmitSpread)
    if(boss_.aimed_fire_timer > 0){
        --boss_.aimed_fire_timer;
    }
    if(boss_.aimed_fire_timer == 0){
        PatternEmitRequest req{};
        req.origin         = boss_.position;
        req.base_angle_rad = GameplayMath::AimAngleRad(boss_.position, player_position);
        req.bullet_count   = AimedBulletCount;
        req.spread_rad     = AimedSpreadRad;
        req.bullet_speed   = AimedBulletSpeed;
        req.bullet_radius  = AimedBulletRadius;
        (void)patterns.EmitSpread(req, enemy_bullets);
        boss_.aimed_fire_timer = boss_.aimed_fire_interval;
    }

    // 全方向弾 (EmitCircle)
    if(boss_.circle_fire_timer > 0){
        --boss_.circle_fire_timer;
    }
    if(boss_.circle_fire_timer == 0){
        PatternEmitRequest req{};
        req.origin         = boss_.position;
        req.base_angle_rad = 0.0f;
        req.bullet_count   = CircleBulletCount;
        req.bullet_speed   = CircleBulletSpeed;
        req.bullet_radius  = CircleBulletRadius;
        (void)patterns.EmitCircle(req, enemy_bullets);
        boss_.circle_fire_timer = boss_.circle_fire_interval;
    }

    return BossEvents::Active;
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
        return BossDamageResult::Destroyed;
    }
    // ダメージ処理
    boss_.hp -= damage;
    return BossDamageResult::Damaged;
}

}   // namespace zimovka
