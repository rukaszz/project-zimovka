#include "zimovka/systems/boss/BossSystem.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <numbers>
#include <stdexcept>

#include "zimovka/config/ScreenLayout.hpp"
#include "zimovka/math/GameplayMath.hpp"
#include "zimovka/systems/pattern/PatternEmitRequest.hpp"

namespace zimovka{
// ボスの移動可能エリア(画面端マージン)
constexpr float MOVE_MARGIN_X = 64.0f;
constexpr float MOVE_MARGIN_Y = 48.0f;
constexpr float MOVE_MAX_Y    = ScreenLayout::PLAYFIELD_HEIGHT * 0.4f;  // 上方40%に限定

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

/**
 * @brief Start()で呼ばれBossConfigのデータを確認する
 *
 * NOTE: ボスごとに項目数などが変わる場合は引数を追加しボスごとに制御するか？
 */
void ValidateBossConfig(const BossConfig& config){
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
                           && config.defeating_duration_ticks > 0;
    if(!valid_radius || !valid_timers){
        throw std::invalid_argument(
            "Invalid Boss config: invalid integer values."
        );
    }
    // BossPhaseDefの検証
    if(config.phases.empty()){
        throw std::invalid_argument(
            "Invalid Boss config: phase definition is empty."
        );
    }
    for(const auto& phase : config.phases){
        // clear_at_hpはフェーズ終了の基準値でありmax_hp以上の場合もNG
        if(phase.max_hp      <= 0
        || phase.clear_at_hp <  0
        || phase.max_hp      <= phase.clear_at_hp){
            throw std::invalid_argument(
                "Invalid HP property. "
            );
        }
        // BossAttackSequenceDef
        if(phase.attack_sequence.empty()){
            throw std::invalid_argument(
                "Boss attack sequence must not be empty. "
            );
        }
        for(const auto& def : phase.attack_sequence){
            if(def.duration_ticks == 0){
                throw std::invalid_argument(
                    "Boss attack step duration must be positive. "
                );
            }
            if(def.step <  BossAttackStep::AimedPressure
            || def.step >= BossAttackStep::Count)
            {
                throw std::invalid_argument(
                    "Boss attack step is not valid. "
                );
            }
            // 発射ステップにはBossShotDefが必須
            if((def.step == BossAttackStep::AimedPressure
            ||  def.step == BossAttackStep::CircleBarrage)
            && !def.shot)
            {
                throw std::invalid_argument(
                    "Firing steps (AimedPressure, CircleBarrage) require BossShotDef. "
                );
            }
            // BossShotDef
            if(def.shot){            
                const auto& boss_shot = *def.shot;
                if(boss_shot.fire_interval_tick == 0){
                    throw std::invalid_argument(
                        "Boss fire interval must be positive. "
                    );
                }
                if(boss_shot.bullet_count == 0
                || boss_shot.fire_interval_tick == 0
                || boss_shot.bullet_speed <= 0.0f
                || !std::isfinite(boss_shot.bullet_speed)
                || boss_shot.bullet_radius < 0.0f || boss_shot.spread_radius < 0.0f
                || !std::isfinite(boss_shot.bullet_radius) || !std::isfinite(boss_shot.spread_radius)
                )
                {
                    throw std::invalid_argument(
                        "Invalid Boss bullet property. "
                    );
                }
            }
            // BossMovementDef
            if(def.movement){
                const auto& move = *def.movement;
                switch(move.type){
                case BossMoveType::Stay:
                case BossMoveType::MoveTo:
                case BossMoveType::Teleportation:
                    break;
                case BossMoveType::Count:
                default:
                    throw std::invalid_argument(
                        "Invalid BossMoveType. "
                    );
                }
                switch(move.target_mode){
                case BossMoveTarget::Fixed:
                case BossMoveTarget::Random:
                case BossMoveTarget::FacePlayer:
                case BossMoveTarget::OppositePlayer:
                    break;
                case BossMoveTarget::Count:
                default:
                    throw std::invalid_argument(
                        "Invalid BossMoveType. "
                    );
                }
                // target_modeに応じて使用フィールドのみ検証
                // Stayの場合は検証は不要
                if(move.type == BossMoveType::Stay){
                    continue;
                }
                // Stay以外の共通的な条件
                if(def.movement->duration_ticks == 0){
                    throw std::invalid_argument(
                        "Invalid movement duration ticks. "
                    );
                }
                // Fixed: 移動先の有効性
                if(def.movement->target_mode == BossMoveTarget::Fixed){
                    if(def.movement->target_position.x <= 0.0f
                    || def.movement->target_position.y <= 0.0f
                    || !IsFinite(def.movement->target_position)
                    )
                    {
                        throw std::invalid_argument(
                            "Invalid Vec2 elements. "
                        );
                    }
                }
                // Random: ランダム移動の範囲検証
                if(def.movement->target_mode == BossMoveTarget::Random){
                    if(def.movement->move_range.x <= 0.0f
                    || def.movement->move_range.y <= 0.0f
                    || !IsFinite(def.movement->move_range)
                    )
                    {
                        throw std::invalid_argument(
                            "Invalid Vec2 elements. "
                        );
                    }
                }
            }
        }
    }
}
}   // namespace

/**
 * @brief ボス戦開始処理
 *
 * @param config
 * @param rng
 */
void BossSystem::Start(const BossConfig& config){
    // Start()はInactiveな状態で一度のみ呼ばれなければならない
    if(boss_.status!= BossStatus::Inactive){
        throw std::logic_error(
            "BossSystem::Start requires Inactive state. "
        );
    }
    // configの検証
    ValidateBossConfig(config);
    // 実行状態を初期化
    boss_ = Boss{};

    // configの適用
    boss_.name        = config.name;

    boss_.position    = config.position;
    boss_.velocity    = config.velocity;
    boss_.render_size = config.render_size;

    boss_.hurtbox_radius = config.hurtbox_radius;
    boss_.contact_radius = config.contact_radius;

    boss_.entering_ticks_remaining    = config.entering_duration_ticks;
    boss_.defeating_duration_ticks    = config.defeating_duration_ticks;
    boss_.defeating_ticks_remaining   = config.defeating_duration_ticks;

    // シーケンスを保持
    boss_.phases = config.phases;
    boss_.hp     = boss_.phases[boss_.phase_index].max_hp;  // phase_index == 0

    // ライフサイクル初期化
    boss_.status = BossStatus::Entering;
}

/**
 * @brief ボスのリセット
 *
 */
void BossSystem::Reset() noexcept{
    boss_ = Boss{};
}

/**
 * @brief 指定したインデックスのフェーズのパターンの定義を読み込む
 *
 */
void BossSystem::ChangePhase(
    std::size_t index, const Vec2& player_position, DeterministicRng& rng
)
{
    boss_.phase_index = index;
    boss_.hp = boss_.phases[index].max_hp;
    // 新フェーズの最初のステップ(0)で開始
    EnterAttackStep(0, player_position, rng);
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
 * @param rng
 * @param patterns
 * @param boss_bullets
 * @return BossTickEvents
 */
BossTickEvents BossSystem::UpdateTick(
    float dt,
    const Vec2& player_position,
    DeterministicRng& rng,
    PatternSystem& patterns,
    BulletSystem& boss_bullets
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
    // Movement
    // move_elapsed_ticksの無制限な増加を防止するために最初に判定
    if(boss_.move_elapsed_ticks < boss_.move_duration_ticks){
        ++boss_.move_elapsed_ticks;
        // 瞬間移動(特定の時間が経ったらtarget_positionへ代入)
        if(boss_.move_type == BossMoveType::Teleportation){
            // elapsed < durationの間は位置を変えない
            if(boss_.move_elapsed_ticks == boss_.move_duration_ticks){
                boss_.position = boss_.move_target; // スナップ
            }
        }
        // 通常の移動(線形移動)
        else if(boss_.move_type == BossMoveType::MoveTo){
            const float t = static_cast<float>(boss_.move_elapsed_ticks)
                            / static_cast<float>(boss_.move_duration_ticks);
            // lerpによる線形補間(2点a, bを時間tで補間する)
            const float x = std::lerp(boss_.move_start.x, boss_.move_target.x, t);
            const float y = std::lerp(boss_.move_start.y, boss_.move_target.y, t);
            boss_.position = {x, y};
        }
    }
    

    // ─── Entering ─────────────────────────────
    // Entering: fight_tick消費 → Fightingへ遷移
    if(boss_.status == BossStatus::Entering){
        // tick消費と残0判定で2回ifが必要
        if(boss_.entering_ticks_remaining > 0){
            --boss_.entering_ticks_remaining;
        }
        if(boss_.entering_ticks_remaining == 0){
            boss_.status = BossStatus::Fighting;
            // EnteringのときにAttackStepを設定する
            EnterAttackStep(0, player_position, rng);
        }
        return {};
    }

    // ─── Fighting ─────────────────────────────
    // 攻撃前に行動フェーズ遷移の確認
    if(boss_.phase_transition_pending){
        boss_.phase_transition_pending = false;
        ChangePhase(boss_.phase_index + 1, player_position, rng);
    }
    // Fighting: パターン発射
    (void)UpdateFighting(player_position, rng, patterns, boss_bullets);

    return {};
}

/**
 * @brief ボスのダメージ処理(処理結果を返す)
 *
 * @param damage
 * @param player_position
 * @param rng
 * @return BossDamageResult
 */
BossDamageResult BossSystem::TakeDamage(std::int32_t damage){
    // Fightingでのみダメージを受ける
    if(damage <= 0 
    || boss_.status != BossStatus::Fighting
    || boss_.phase_transition_pending   // フェーズ遷移中のダメージは無効
    )
    {
        return BossDamageResult::InvalidTarget;
    }
    const auto& phase = GetPhase();
    const int   damage_needed = boss_.hp - phase.clear_at_hp;
    
    // clear_at_hpに到達しないならダメージを与えるのみ
    if(damage < damage_needed){
        // ダメージ処理
        boss_.hp -= damage;
        return BossDamageResult::Damaged;
    }
    // ここに来たらフェーズ遷移
    boss_.hp = phase.clear_at_hp;
    // 最後のフェーズではphase_indexを加算すると範囲外アクセスとなるので一度変数で受ける
    const std::size_t next_index = boss_.phase_index + 1;
    // 最終フェーズの場合phase_indexは加算せずDestroyed
    if(next_index >= boss_.phases.size()){
        boss_.hp = 0;
        boss_.status = BossStatus::Defeating;
        boss_.defeating_ticks_remaining = boss_.defeating_duration_ticks;
        return BossDamageResult::Destroyed;
    }
    // 次のフェーズへ遷移
    // ChangePhase(boss_.phase_index, player_position, rng);
    boss_.phase_transition_pending = true;  // フェーズ遷移を予約
    return BossDamageResult::PhaseComplete;
}

/**
 * @brief ボスの攻撃パターン制御
 *
 * attack_step_ticksによるBossAttackStepの切り替え
 */
void BossSystem::AdvanceAttackStep(
    const Vec2& player_position, 
    DeterministicRng& rng
)
{
    // 攻撃パターンの定義取得
    const std::size_t next =
        (boss_.attack_step_index + 1) % GetPhase().attack_sequence.size();
    EnterAttackStep(next, player_position, rng);
}

/**
 * @brief ボスが攻撃状態に入った際に発射タイマーをリセットする
 *
 * @param index
 * @param player_position
 * @param rng
 */
void BossSystem::EnterAttackStep(
    std::size_t index, const Vec2& player_position, DeterministicRng& rng
)
{
    // 攻撃状態の定義を取得
    boss_.attack_step_index = index;
    const auto& def = GetPhase().attack_sequence[index];

    boss_.attack_step = def.step;
    boss_.attack_step_ticks_remaining = def.duration_ticks;
    // 発射ステップはValidationでBossShotDef必須を保証済み
    // 非発射ステップ(SpecialWarning, Recovery)はfire_timerを使用しないため0で初期化
    boss_.fire_timer = def.shot ? def.shot->fire_interval_tick : 0;
    
    CalculateMovement(def, player_position, rng);
}

/**
 * @brief ボスの移動計算
 *
 * BossAttackSequenceDefのmovementフィールドに基づき，
 * 次の移動先を計算してboss_の移動ランタイム状態を更新する
 *
 * @param seq
 * @param player_position
 * @param rng
 */
void BossSystem::CalculateMovement(
    const BossAttackSequenceDef& seq,
    const Vec2& player_position,
    DeterministicRng& rng
)
{
    // optionalが有効な値を保持しているか
    // ※has_valueを用いるとnulloptのときも移動が継続するので用いない
    if(!seq.movement){
        // 移動の状態を初期へ戻す
        boss_.move_type = BossMoveType::Stay;
        boss_.move_duration_ticks = 0;
        boss_.move_elapsed_ticks  = 0;
        return;
    }
    const auto& move          = *seq.movement;
    boss_.move_start          = boss_.position;
    boss_.move_type           = move.type;
    boss_.move_duration_ticks = move.duration_ticks;
    boss_.move_elapsed_ticks  = 0;

    // 最初に移動タイプを見る
    switch(move.type){
    case BossMoveType::Stay:
        boss_.move_duration_ticks = 0;
        boss_.move_elapsed_ticks  = 0;
        return;
    case BossMoveType::MoveTo:
    case BossMoveType::Teleportation:
        break;  // 下の処理へ進む
    default:
        throw std::logic_error(
            "Unsupported BossMoveType. "
        );
    }

    switch(move.target_mode){
    case BossMoveTarget::Fixed:
        boss_.move_target = move.target_position;
        break;

    case BossMoveTarget::Random:{
        // move_rangeをプレイフィールド中央上部を基点とした±範囲として扱う
        const float cx = ScreenLayout::PLAYFIELD_WIDTH  * 0.5f;
        const float cy = ScreenLayout::PLAYFIELD_HEIGHT * 0.2f;
        // 移動範囲の最大最小
        const float min_x = std::max(MOVE_MARGIN_X, cx - move.move_range.x);
        const float max_x = std::min(ScreenLayout::PLAYFIELD_WIDTH  - MOVE_MARGIN_X, cx + move.move_range.x);
        const float min_y = std::max(MOVE_MARGIN_Y, cy - move.move_range.y);
        const float max_y = std::min(MOVE_MAX_Y,    cy + move.move_range.y);
        boss_.move_target = {
            rng.UniformFloat(min_x, max_x),
            rng.UniformFloat(min_y, max_y)
        };
        break;
    }

    case BossMoveTarget::OppositePlayer:
        // X軸をプレイヤーの反対側・Yは上方25%付近に固定
        boss_.move_target = {
            std::clamp(
                ScreenLayout::PLAYFIELD_WIDTH - player_position.x,
                MOVE_MARGIN_X,
                ScreenLayout::PLAYFIELD_WIDTH - MOVE_MARGIN_X
            ),
            ScreenLayout::PLAYFIELD_HEIGHT * 0.25f
        };
        break;

    case BossMoveTarget::FacePlayer:
        // プレイヤーと同じX座標・Yは上方25%付近に固定
        boss_.move_target = {
            std::clamp(
                player_position.x,
                MOVE_MARGIN_X,
                ScreenLayout::PLAYFIELD_WIDTH - MOVE_MARGIN_X
            ),
            ScreenLayout::PLAYFIELD_HEIGHT * 0.25f
        };
        break;
    }
}

/**
 * @brief 自機狙い扇形弾
 *
 * @param player_position
 * @param patterns
 * @param boss_bullets
 * @return std::size_t 発射した弾数
 */
std::size_t BossSystem::UpdateAimedAttack(
    const Vec2& player_position,
    PatternSystem& patterns,
    BulletSystem& boss_bullets
)
{
    const auto& shot = *CurrentStep().shot;
    PatternEmitRequest req{};
    req.origin         = boss_.position;
    req.base_angle_rad = GameplayMath::AimAngleRad(boss_.position, player_position);
    req.bullet_count   = shot.bullet_count;
    req.spread_rad     = shot.spread_radius;
    req.bullet_speed   = shot.bullet_speed;
    req.bullet_radius  = shot.bullet_radius;
    return patterns.EmitSpread(req, boss_bullets);
}

/**
 * @brief 全包囲弾発射
 *
 * @param patterns
 * @param boss_bullets
 * @return std::size_t
 */
std::size_t BossSystem::UpdateCircleAttack(
    PatternSystem& patterns,
    BulletSystem& boss_bullets
)
{
    const auto& shot = *CurrentStep().shot;
    PatternEmitRequest req{};
    req.origin         = boss_.position;
    req.base_angle_rad = 0.0f;
    req.bullet_count   = shot.bullet_count;
    req.bullet_speed   = shot.bullet_speed;
    req.bullet_radius  = shot.bullet_radius;
    return patterns.EmitCircle(req, boss_bullets);
}

/**
 * @brief 戦闘時のパターン別攻撃制御
 *
 * BossStatus::Fightingの際に実行される
 * 契約として，残り時間0になった次の更新で状態を移行する
 *
 * @param player_position
 * @param rng
 * @param patterns
 * @param boss_bullets
 */
void BossSystem::UpdateFighting(
    const Vec2& player_position,
    DeterministicRng& rng,
    PatternSystem& patterns,
    BulletSystem& boss_bullets
)
{
    // 攻撃ステップの残時間0で次の攻撃状態へ
    if(boss_.attack_step_ticks_remaining == 0){
        AdvanceAttackStep(player_position, rng);
    }
    // BossAttackStepに応じて行動パターンを変える
    switch(boss_.attack_step){
    // 自機狙い弾攻撃
    case BossAttackStep::AimedPressure:
        // 自機狙い拡散弾 (EmitSpread)
        if(boss_.fire_timer > 0){
            --boss_.fire_timer;
        }
        // configのValidateで防御するためhas_valueは不要
        if(boss_.fire_timer == 0){
            UpdateAimedAttack(player_position, patterns, boss_bullets);
            boss_.fire_timer = CurrentStep().shot->fire_interval_tick;
        }
        break;
    // 形態移行中
    case BossAttackStep::SpecialWarning:
        break;  // 攻撃しない
    // 全包囲攻撃
    case BossAttackStep::CircleBarrage:
        // 全方向弾 (EmitCircle)
        if(boss_.fire_timer > 0){
            --boss_.fire_timer;
        }
        // configのValidateで防御するためhas_valueは不要
        if(boss_.fire_timer == 0){
            UpdateCircleAttack(patterns, boss_bullets);
            boss_.fire_timer = CurrentStep().shot->fire_interval_tick;
        }
        break;
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
