#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

#include "timing_stats.hpp"
#include "zimovka/config/SimulationConfig.hpp"
#include "zimovka/core/DeterministicRng.hpp"
#include "zimovka/engine/update/UpdatePipeline.hpp"
#include "zimovka/events/GameplayTickEvents.hpp"
#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/replay/RunPlayback.hpp"
#include "zimovka/replay/RunRecorder.hpp"

using zimovka::ActionBit;
using zimovka::DeterministicRng;
using zimovka::GameplayTickEvents;
using zimovka::InputState;
using zimovka::PlaybackStartResult;
using zimovka::RunPlayback;
using zimovka::RunRecorder;
using zimovka::UpdatePipeline;

// ──────────────────────────────────────────────────────────
// ヘルパ / シナリオ定義 (全て無名名前空間内に格納)
// ──────────────────────────────────────────────────────────
namespace{

// ── ワールド設定 ───────────────────────────────────────────
constexpr float WORLD_W = 960.0f;
constexpr float WORLD_H = 720.0f;
constexpr float DT      = zimovka::SimulationConfig::FIXED_DELTA_SECONDS;

// ── シナリオスクリプト定義 ─────────────────────────────────
/**
 * @brief シナリオの1ステップ
 *
 * duration_ticks : このステップが継続するTick数
 * held_bits      : 押し続けるキー(移動方向など)
 * want_shoot     : このstepで12tick間隔のShoot入力を生成するか
 */
struct ScenarioStep{
    std::uint32_t duration_ticks;
    std::uint32_t held_bits;
    bool          want_shoot;
};

/**
 * @brief Phase0 シナリオスクリプト (合計360tick = 6秒)
 *
 * PlayerWeaponConfig デフォルト値:
 *   max_ammo=6, shot_cooldown_ticks=8, reload_duration_ticks=90
 *
 * 各ステップをインデックスで与え移動・射撃の組み合わせを網羅し，
 * 衝突/撃破/リロード/再発射が1シナリオに含まれるようにしている
 */
static constexpr std::array<ScenarioStep, 6> phase0_script = {{
    // ステップ0: 右移動 + 射撃(60tick)
    //     初弾が20tick前後で敵に命中, 6発打ち切り後にリロード開始
    {60u, ActionBit(zimovka::Action::MoveRight), true },
    // ステップ1: 左移動 + 射撃(60tick)
    //     リロード継続(90tick), 完了後に弾が再補充されて発射再開
    {60u, ActionBit(zimovka::Action::MoveLeft),  true },
    // ステップ2: 静止 + 射撃 (60tick)
    {60u, 0u,                                    true },
    // ステップ3: 右移動 + 低速(60tick, 射撃なし)
    //     Shoftで低速移動, Shootなし → 弾数変化なし
    {60u, ActionBit(zimovka::Action::MoveRight)
        | ActionBit(zimovka::Action::Slow),      false},
    // ステップ4: 左移動 + 射撃 (60tick)
    {60u, ActionBit(zimovka::Action::MoveLeft),  true },
    // ステップ5: 静止(60 tick, 射撃なし)
    {60u, 0u,                                    false},
}};

// ── Phase0InputScript ───────────────────────────────────
/**
 * @brief Phase0用のシナリオ入力生成クラス
 *
 * UpdatePipelineを汚染しないよう外部クラスとして定義している．
 * UpdatePipeline::UpdateTick()へ渡すInputStateのみを生成する．
 *
 *
 * これによって，ステップ切り替えやwant_shootフラグの値が変わっても
 * 「毎tick N+k回目の消費が行われる」という乱数の消費が固定で保たれる.
 * → 同一Seedでは必ず同一の入力列が再現される.
 */
class Phase0InputScript{
private:
    std::size_t   current_step_  = 0;
    std::uint32_t tick_in_step_  = 0;
    std::uint32_t previous_held_ = 0;
public:
    explicit Phase0InputScript() = default;

    /**
     * @brief シードをリセットしてシナリオを最初から再生する
     * テストの「同一Seedで同一列」検証に使用する
     *
     */
    void Reset() noexcept{
        current_step_  = 0;
        tick_in_step_  = 0;
        previous_held_ = 0;
    }

    /**
     * @brief 1tick分のInputStateを生成し内部Tickを進める
     *
     * RNG消費: want_shootに関わらずUnitFloat()を必ず1回消費する
     */
    [[nodiscard]]
    InputState GenerateNextInput(){
        // 実施するスクリプト
        const ScenarioStep& step = phase0_script[current_step_];

        // 押し続ける入力を保持
        std::uint32_t held = step.held_bits;

        // 12Tickごとに1TickだけShootを入力(武器システムはIsPressed=just-pressed入力で発射)
        if(step.want_shoot && tick_in_step_ % 12u == 0u){
            held |= ActionBit(zimovka::Action::Shoot);
        }

        // 今のTickで押された & 前回押されていない(前回押されたの否定) = pressed
        const std::uint32_t pressed = held & ~previous_held_;

        // 前回のTickで押された & 今回押されていない(今回押されているの否定) = released
        const std::uint32_t released = previous_held_ & ~held;

        previous_held_ = held;

        // 内部Tickを進める(スクリプトで定義した間隔をすぎるまで，スクリプトのステップはそのまま)
        if(++tick_in_step_ >= step.duration_ticks){
            ++current_step_;
            tick_in_step_ = 0;
        }

        return InputState::FromBits(held, pressed, released);
    }

    bool IsFinished() const noexcept{
        return current_step_ >= phase0_script.size();
    }

    std::size_t GetCurrentStep() const noexcept{
        return current_step_;
    }

    // シナリオの総Tick数
    static constexpr std::uint32_t TotalTicks() noexcept{
        std::uint32_t total = 0;
        for(const auto& s : phase0_script){
            total += s.duration_ticks;
        }
        return total;
    }
};

// ── Phase0CountAccumulation ─────────────────────────────────────
/**
 * @brief シナリオ終了後に比較するための累積値
 * 
 * 両方ゼロ(Play/Replay)の場合で正常と判定されないように累積値を用いる
 */
struct GameLikeTotals{
    std::uint32_t shots_fired = 0;
    std::uint32_t reload_started = 0;
    std::uint32_t reload_completed = 0;

    std::uint32_t enemy_hits = 0;
    std::uint32_t enemy_kills = 0;

    std::uint32_t player_hits = 0;
};

// ── Phase0TickSnapshot ─────────────────────────────────────
/**
 * @brief 1tick の観測可能な状態のスナップショット
 *
 * UpdatePipelineの公開APIから取得できるフィールドのみで構成する.
 * state_hashは全フィールドの複合ハッシュ.
 * update_nsは処理時間(gtestでは比較しない).
 */
struct Phase0TickSnapshot{
    std::uint64_t tick                = 0;
    // 弾プール状態
    std::uint32_t player_bullet_count = 0;
    std::uint32_t enemy_bullet_count  = 0;
    // 武器状態
    std::uint32_t weapon_ammo         = 0;
    std::uint32_t weapon_cooldown     = 0;
    std::uint32_t weapon_reload       = 0;
    // Tickイベント
    bool          player_hit          = false;
    std::uint32_t enemy_hit_count     = 0;
    std::uint32_t enemy_kill_count    = 0;
    bool          shot_fired          = false;
    // 複合ハッシュ
    // size_tはプラットフォーム依存なのでuint64_t
    std::uint64_t state_hash          = 0;  // RNG状態含む完全ハッシュ(Play/Replay決定論性用)
    std::uint64_t world_hash          = 0;  // RNG除外ワールド状態ハッシュ(異Seed検証用)
    // 処理時間 [ns] ※gtestではEXPECTせず記録して比較する
    std::int64_t  update_ns           = 0;
};

// ── State Hash ─────────────────────────────────────────────
/**
 * @brief Replay決定性比較用の簡易64bit state hash※32ビット値をFNV-1a風に混合する
 * 
 * ※FNV-1a: 乗算と排他的論理和(XOR)のみを使用する非常にシンプルで高速な非暗号化ハッシュ関数
 * 
 * @param h: 累積ハッシュ値 
 * @param v: 混ぜ込みたい32ビット値
 * @return std::uint64_t
 */
static std::uint64_t HashMix(std::uint64_t h, std::uint32_t v) noexcept{
    h ^= static_cast<std::uint64_t>(v);
    h *= 0x9e3779b97f4a7c15ULL; // bitsの拡散を助ける黄金比の小数部分の2乗(0.6180...^2)のマジックナンバー
    return h;
}

/**
 * @brief floatをbit-castしてHashMixに渡す
 * 
 * @param h: 累積ハッシュ値
 * @param f: float値※memcpyでビットをコピーするのでビットの暗黙的な変換は発生しない 
 * @return * std::uint64_t 
 */
static std::uint64_t HashMixFloat(std::uint64_t h, float f) noexcept{
    std::uint32_t bits = 0;
    std::memcpy(&bits, &f, sizeof(bits));   // fのメモリ上のビット表現をそのままbitsへ渡す
    return HashMix(h, bits);
}

/**
 * @brief UpdatePipelineから観測可能な状態を取得し複合ハッシュを計算する
 *
 * 対象:
 *   - tick_index_(パイプラインTick)
 *   - Player: position, hit_radius
 *   - Enemy per-slot: active/position/velocity/hp/hurtbox_radius
 *   - 自機弾 per-slot: slot番号/active/position/velocity/radius + next_spawn_idx_
 *   - 敵弾 per-slot: slot番号/active/position/velocity/radius + next_spawn_idx_
 *   - 武器状態: ammo/cooldown/reload
 *   - Tickイベント: player_hit/enemy_hit/kill/shot_fired等
 *   - 衝突判定カウント
 *   - RNG状態: seed/draw_count_
 */
/**
 * @brief ゲーム状態のFNV-1a 64bit風の複合ハッシュを計算する
 *
 * @param include_rng trueのとき gameplay_rng_のseed/draw_count_もハッシュに含める.
 *   - true  → state_hash: RNGを含む完全な状態ハッシュ(Play/Replayの決定論性検証に使用)
 *   - false → world_hash: RNG除外のゲームワールド状態ハッシュ(異Seed時に世界が実際に
 *              異なることを検証するために使用. RNG値を直接含まないため trivial な成功を防ぐ)
 */
static std::uint64_t ComputeStateHash(
    const UpdatePipeline&     pipeline,
    const GameplayTickEvents& events,
    bool                      include_rng = true
) noexcept {
    // 累積ハッシュ値
    std::uint64_t h = 0xcbf29ce484222325ULL;

    // ── Pipeline: tick_index_ ───────────────────────────────
    const std::uint64_t tick = pipeline.GetTickIndex();
    h = HashMix(h, static_cast<std::uint32_t>(tick));
    h = HashMix(h, static_cast<std::uint32_t>(tick >> 32));

    // ── Player ──────────────────────────────────────────────
    const auto& player = pipeline.GetPlayerSystem().GetPlayer();
    h = HashMixFloat(h, player.position.x);
    h = HashMixFloat(h, player.position.y);
    h = HashMixFloat(h, player.hit_radius);

    // ── Enemy per-slot ───────────────────────────────────────
    for(const auto& e : pipeline.GetEnemySystem().GetEnemies()){
        h = HashMix(h, e.active ? 1u : 0u);
        h = HashMixFloat(h, e.position.x);
        h = HashMixFloat(h, e.position.y);
        h = HashMixFloat(h, e.velocity.x);
        h = HashMixFloat(h, e.velocity.y);
        h = HashMixFloat(h, e.render_size.x);
        h = HashMixFloat(h, e.render_size.y);
        h = HashMix(h, static_cast<std::uint32_t>(e.hp));
        h = HashMixFloat(h, e.hurtbox_radius);
    }
    // ── EnemySystem システムレベルの状態 ─────────────────────
    // next_spawn_index_: 次スポーンで使うスロット番号(将来の状態に影響)
    // active_count_    : スロットループ外からの一括チェック用
    h = HashMix(h, static_cast<std::uint32_t>(pipeline.GetEnemySystem().CountActive()));
    h = HashMix(h, static_cast<std::uint32_t>(pipeline.GetEnemySystem().GetNextSpawnIndex()));

    // ── 自機弾 per-slot ──────────────────────────────────────
    {
        const auto& pb = pipeline.GetPlayerBullets();
        h = HashMix(h, static_cast<std::uint32_t>(pb.CountActive()));
        h = HashMix(h, static_cast<std::uint32_t>(pb.GetNextSpawnIdx()));
        std::uint32_t slot = 0u;
        for(const auto& b : pb.GetBullets()){
            h = HashMix(h, slot);
            h = HashMix(h, b.active ? 1u : 0u);
            h = HashMixFloat(h, b.position.x);
            h = HashMixFloat(h, b.position.y);
            h = HashMixFloat(h, b.velocity.x);
            h = HashMixFloat(h, b.velocity.y);
            h = HashMixFloat(h, b.radius);
            ++slot;
        }
    }

    // ── 敵弾 per-slot ────────────────────────────────────────
    {
        const auto& eb = pipeline.GetEnemyBullets();
        h = HashMix(h, static_cast<std::uint32_t>(eb.CountActive()));
        h = HashMix(h, static_cast<std::uint32_t>(eb.GetNextSpawnIdx()));
        std::uint32_t slot = 0u;
        for(const auto& b : eb.GetBullets()){
            h = HashMix(h, slot);
            h = HashMix(h, b.active ? 1u : 0u);
            h = HashMixFloat(h, b.position.x);
            h = HashMixFloat(h, b.position.y);
            h = HashMixFloat(h, b.velocity.x);
            h = HashMixFloat(h, b.velocity.y);
            h = HashMixFloat(h, b.radius);
            ++slot;
        }
    }

    // ── 武器状態 ─────────────────────────────────────────────
    const auto& ws = pipeline.GetPlayerWeaponSystem().GetState();
    h = HashMix(h, ws.ammo);
    h = HashMix(h, ws.cooldown_ticks_remaining);
    h = HashMix(h, ws.reload_ticks_remaining);

    // ── Tickイベント ─────────────────────────────────────────
    // ハッシュ累積値
    h = HashMix(h, events.player_hit              ? 1u : 0u);
    h = HashMix(h, events.weapon.shot_fired       ? 1u : 0u);
    h = HashMix(h, events.weapon.reload_started   ? 1u : 0u);
    h = HashMix(h, events.weapon.reload_completed ? 1u : 0u);
    h = HashMix(h, static_cast<std::uint32_t>(events.enemy_hit.hit_count));
    h = HashMix(h, static_cast<std::uint32_t>(events.enemy_hit.kill_count));

    // ── 衝突判定カウント ──────────────────────────────────────
    const auto& cs = pipeline.GetCollisionSystem().GetStats();
    h = HashMix(h, static_cast<std::uint32_t>(cs.player_vs_enemy_bullet_checks));
    h = HashMix(h, static_cast<std::uint32_t>(cs.player_bullet_vs_enemy_checks));

    // ── RNG状態(gameplay_rng_) ───────────────────────────────
    // include_rng=falseのとき省略 → world_hashとして使用
    if(include_rng){
        h = HashMix(h, pipeline.GetRngSeed());
        const std::uint64_t draw = pipeline.GetRngDrawCount();
        h = HashMix(h, static_cast<std::uint32_t>(draw));
        h = HashMix(h, static_cast<std::uint32_t>(draw >> 32));
    }

    return h;
}

/**
 * @brief UpdateTick()後の状態からPhase0TickSnapshotを構築する
 * 
 */
static Phase0TickSnapshot ComputeSnapshot(
    std::uint64_t             tick,
    const UpdatePipeline&     pipeline,
    const GameplayTickEvents& events,
    std::int64_t              update_ns
) noexcept {
    const auto& ws = pipeline.GetPlayerWeaponSystem().GetState();
    return Phase0TickSnapshot{
        .tick                = tick,
        .player_bullet_count = static_cast<std::uint32_t>(
                                   pipeline.GetPlayerBullets().CountActive()
                                ),
        .enemy_bullet_count  = static_cast<std::uint32_t>(
                                   pipeline.GetEnemyBullets().CountActive()
                                ),
        .weapon_ammo         = ws.ammo,
        .weapon_cooldown     = ws.cooldown_ticks_remaining,
        .weapon_reload       = ws.reload_ticks_remaining,
        .player_hit          = events.player_hit,
        .enemy_hit_count     = static_cast<std::uint32_t>(events.enemy_hit.hit_count),
        .enemy_kill_count    = static_cast<std::uint32_t>(events.enemy_hit.kill_count),
        .shot_fired          = events.weapon.shot_fired,
        .state_hash          = ComputeStateHash(pipeline, events, true),
        .world_hash          = ComputeStateHash(pipeline, events, false),
        .update_ns           = update_ns,
    };
}

// ── 実行ヘルパ ─────────────────────────────────────────────
/**
 * @brief GameLikeTotalsに1Tickのイベントを加算する
 * 
 */
static void AccumulateTick(GameLikeTotals& t, const GameplayTickEvents& events) noexcept{
    // 発射回数
    if(events.weapon.shot_fired)       ++t.shots_fired;
    // リロード回数
    if(events.weapon.reload_started)   ++t.reload_started;
    // リロード完了回数
    if(events.weapon.reload_completed) ++t.reload_completed;
    // 敵の被弾
    t.enemy_hits  += static_cast<std::uint32_t>(events.enemy_hit.hit_count);
    t.enemy_kills += static_cast<std::uint32_t>(events.enemy_hit.kill_count);
    
    // 被弾回数
    if(events.player_hit)              ++t.player_hits;
}

/**
 * @brief プレイフェーズ: シナリオを1回実行してスナップショットを収集する
 *
 * @tparam ScenarioT  IsFinished()/GenerateNextInput() を持つシナリオ型
 * @param pipeline    StartRun()済みのUpdatePipeline
 * @param scenario    実行するシナリオ
 * @param recorder    Start()済みのRunRecorder (nullptr可 → 記録しない)
 * @param totals_out  累積値の出力先 (nullptr可 → 集計しない)
 */
template<typename ScenarioT>
static std::vector<Phase0TickSnapshot> RunPlayPhase(
    UpdatePipeline& pipeline,
    ScenarioT&      scenario,
    RunRecorder*    recorder,
    GameLikeTotals* totals_out = nullptr
)
{
    std::vector<Phase0TickSnapshot> snaps;
    // スクリプトの規定Tick回数ループ
    for(std::uint64_t tick = 0; !scenario.IsFinished(); ++tick){
        const InputState input = scenario.GenerateNextInput();
        if(recorder){
            recorder->Record(input);
        }

        const auto t0     = std::chrono::steady_clock::now();
        const auto events = pipeline.UpdateTick(DT, input);
        const auto t1     = std::chrono::steady_clock::now();
        // t1-t0の結果がnanosecondとは限らないのでキャスト
        const auto update_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

        if(totals_out){
            AccumulateTick(*totals_out, events);
        }
        snaps.push_back(ComputeSnapshot(tick, pipeline, events, update_ns));
    }
    return snaps;
}

/**
 * @brief リプレイフェーズ: RunPlaybackから入力を取り出してスナップショットを収集する
 *
 * @param pipeline   StartRun()済みのUpdatePipeline
 * @param playback   Start()済みのRunPlayback
 * @param totals_out 累積値の出力先 (nullptr可 → 集計しない)
 */
static std::vector<Phase0TickSnapshot> RunReplayPhase(
    UpdatePipeline& pipeline,
    RunPlayback&    playback,
    GameLikeTotals* totals_out = nullptr
){
    std::vector<Phase0TickSnapshot> snaps;
    // 記録したTick回ループ
    for(std::uint64_t tick = 0; !playback.IsFinished(); ++tick){
        const auto opt = playback.ConsumeNextInput();
        EXPECT_TRUE(opt.has_value()) << "tick=" << tick << " ConsumeNextInput()失敗";
        if(!opt.has_value()){
            break;
        }
        const auto t0     = std::chrono::steady_clock::now();
        const auto events = pipeline.UpdateTick(DT, *opt);
        const auto t1     = std::chrono::steady_clock::now();
        // t1-t0の結果がnanosecondとは限らないのでキャスト
        const auto update_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

        if(totals_out){
            AccumulateTick(*totals_out, events);
        }
        snaps.push_back(ComputeSnapshot(tick, pipeline, events, update_ns));
    }
    return snaps;
}

// ── SoakScenarioSystem ─────────────────────────────────────
/**
 * @brief 3600Tick(60秒相当)のソークテスト用シナリオ
 *
 * phase0_scriptを上限なく循環させる.
 * reload完了/spawn/float更新/tick_index_の累積ズレを長時間で検出する.
 */
class SoakScenarioSystem{
private:
    std::size_t   current_step_  = 0;   // サイクリック(上限なし, mod参照)
    std::uint32_t tick_in_step_  = 0;
    std::uint32_t previous_held_ = 0;
    std::uint32_t total_emitted_ = 0;
public:
    static constexpr std::uint32_t TOTAL_TICKS = 3600u;    // 60秒相当

    explicit SoakScenarioSystem() = default;

    [[nodiscard]]
    InputState GenerateNextInput(){
        const ScenarioStep& step =
            phase0_script[current_step_ % phase0_script.size()];

        // 入力取得
        std::uint32_t held = step.held_bits;
        // 12Tickごとに1TickだけShootを入力(武器システムはIsPressed=just-pressed入力で発射)
        if(step.want_shoot && tick_in_step_ % 12u == 0u){
            held |= ActionBit(zimovka::Action::Shoot);
        }
        const std::uint32_t pressed  = held & ~previous_held_;
        const std::uint32_t released = previous_held_ & ~held;
        previous_held_ = held;
        // シナリオを進めるか判定
        if(++tick_in_step_ >= step.duration_ticks){
            ++current_step_;
            tick_in_step_ = 0;
        }
        ++total_emitted_;
        return InputState::FromBits(held, pressed, released);
    }

    bool IsFinished() const noexcept{
        return total_emitted_ >= TOTAL_TICKS;
    }
};

} // namespace

// ──────────────────────────────────────────────────────────
// Phase0 Game-like 複合試験
// ──────────────────────────────────────────────────────────
/**
 * @brief プレイ→リプレイで全Tickのstate_hashが一致することを確認
 *
 * 手順:
 *   1. Phase0InputScript()で入力列を生成しながらプレイを実行
 *      RunRecorderで入力を記録，各Tickのスナップショットを収集
 *   2. RunPlaybackで記録を再生しながらリプレイを実行，スナップショットを収集
 *   3. 全Tickのstate_hashと個別フィールドが一致することを確認
 *
 * 処理時間(update_ns)はスナップショットに記録するが EXPECT_* では比較しない
 * 
 * Play:
 * record.seed -> UpdatePipeline
 * Replay:
 * record.seed -> UpdatePipeline
 * という対称性を持たせる
 */
TEST(GameLikePhase0Test, PlayAndReplayProduceSameStateHash){
    constexpr DeterministicRng::Seed SEED = 42u;

    // ── プレイフェーズ ──────────────────────────────────────
    UpdatePipeline    play_pipeline;
    Phase0InputScript scenario{};
    RunRecorder       recorder;

    recorder.Start(SEED);
    play_pipeline.StartRun(WORLD_W, WORLD_H, SEED);

    GameLikeTotals play_totals{};
    const auto play_snaps = RunPlayPhase(play_pipeline, scenario, &recorder, &play_totals);

    recorder.Stop();
    ASSERT_FALSE(recorder.GetRecord().frames.empty())
        << "RunRecorderが空: 入力が記録されていない";
    ASSERT_EQ(recorder.GetRecord().frames.size(), play_snaps.size())
        << "記録フレーム数とスナップショット数が不一致";

    // ── リプレイフェーズ ────────────────────────────────────
    // 記録したシード値を取得するためのrecord
    const auto& record = recorder.GetRecord();
    // 記録したシードが違っていたら即終了
    ASSERT_EQ(record.random_seed, SEED);
    UpdatePipeline replay_pipeline;
    // ここでSEEDを使うのはNG
    replay_pipeline.StartRun(WORLD_W, WORLD_H, record.random_seed);

    RunPlayback playback;
    ASSERT_EQ(
        playback.Start(recorder.GetRecord()),
        PlaybackStartResult::Started
    ) << "RunPlayback::Start() が失敗";

    GameLikeTotals replay_totals{};
    const auto replay_snaps = RunReplayPhase(replay_pipeline, playback, &replay_totals);

    // ── 比較フェーズ ────────────────────────────────────────
    ASSERT_EQ(play_snaps.size(), replay_snaps.size())
        << "プレイとリプレイでTick数が異なる";

    for(std::size_t i = 0; i < play_snaps.size(); ++i){
        const auto& p = play_snaps[i];
        const auto& r = replay_snaps[i];

        // state_hash が一致すれば観測可能な全フィールドが一致
        EXPECT_EQ(p.state_hash, r.state_hash)
            << "state_hash 不一致 tick=" << p.tick;

        // ハッシュ衝突による偽陰性を排除するため個別フィールドも検証
        EXPECT_EQ(p.player_bullet_count, r.player_bullet_count)
            << "player_bullet_count 不一致 tick=" << p.tick;
        EXPECT_EQ(p.enemy_bullet_count,  r.enemy_bullet_count)
            << "enemy_bullet_count 不一致 tick=" << p.tick;
        EXPECT_EQ(p.weapon_ammo,         r.weapon_ammo)
            << "weapon_ammo 不一致 tick=" << p.tick;
        EXPECT_EQ(p.weapon_cooldown,     r.weapon_cooldown)
            << "weapon_cooldown 不一致 tick=" << p.tick;
        EXPECT_EQ(p.weapon_reload,       r.weapon_reload)
            << "weapon_reload 不一致 tick=" << p.tick;
        EXPECT_EQ(p.player_hit,          r.player_hit)
            << "player_hit 不一致 tick=" << p.tick;
        EXPECT_EQ(p.enemy_hit_count,     r.enemy_hit_count)
            << "enemy_hit_count 不一致 tick=" << p.tick;
        EXPECT_EQ(p.enemy_kill_count,    r.enemy_kill_count)
            << "enemy_kill_count 不一致 tick=" << p.tick;
        EXPECT_EQ(p.shot_fired,          r.shot_fired)
            << "shot_fired 不一致 tick=" << p.tick;
    }

    // ── 累積値: 非ゼロ検証 (両方ゼロでの偽陰性を排除) ────────
    EXPECT_GT(play_totals.shots_fired,      0u) << "1発も発射されなかった(偽陰性の恐れ)";
    EXPECT_GT(play_totals.reload_started,   0u) << "一度もリロード開始しなかった";
    EXPECT_GT(play_totals.reload_completed, 0u) << "一度もリロード完了しなかった";
    // NOTE: enemy_kills は SpawnPhase0EnemyIfNeeded()の乱数依存で360tickでは保証困難
    //       → 撃破ゼロ検証は SoakTest_3600Ticks で実施する

    // ── 累積値: Play == Replay ──────────────────────────────
    EXPECT_EQ(play_totals.shots_fired,      replay_totals.shots_fired)
        << "shots_fired 累積値が不一致";
    EXPECT_EQ(play_totals.reload_started,   replay_totals.reload_started)
        << "reload_started 累積値が不一致";
    EXPECT_EQ(play_totals.reload_completed, replay_totals.reload_completed)
        << "reload_completed 累積値が不一致";
    EXPECT_EQ(play_totals.enemy_hits,       replay_totals.enemy_hits)
        << "enemy_hits 累積値が不一致";
    EXPECT_EQ(play_totals.enemy_kills,      replay_totals.enemy_kills)
        << "enemy_kills 累積値が不一致";
    EXPECT_EQ(play_totals.player_hits,      replay_totals.player_hits)
        << "player_hits 累積値が不一致";

    // ──── タイミング統計(処理時間は比較せず参考値として記録のみ) ────
    {
        std::vector<std::int64_t> play_ns, replay_ns;
        play_ns.reserve(play_snaps.size());
        replay_ns.reserve(replay_snaps.size());
        for(const auto& s : play_snaps)   play_ns.push_back(s.update_ns);
        for(const auto& s : replay_snaps) replay_ns.push_back(s.update_ns);

        const auto ps = test_util::TimingStats::Compute(play_ns);
        const auto rs = test_util::TimingStats::Compute(replay_ns);

        RecordProperty("gamelike_play_avg_us",    ps.avg_ns / 1000);
        RecordProperty("gamelike_play_p55_us",    ps.p55_ns / 1000);
        RecordProperty("gamelike_play_p99_us",    ps.p99_ns / 1000);
        RecordProperty("gamelike_play_max_us",    ps.max_ns / 1000);
        RecordProperty("gamelike_replay_avg_us",  rs.avg_ns / 1000);
        RecordProperty("gamelike_replay_p55_us",  rs.p55_ns / 1000);
        RecordProperty("gamelike_replay_p99_us",  rs.p99_ns / 1000);
        RecordProperty("gamelike_replay_max_us",  rs.max_ns / 1000);
        RecordProperty("play_shots_fired",     static_cast<int>(play_totals.shots_fired));
        RecordProperty("play_enemy_kills",     static_cast<int>(play_totals.enemy_kills));
        RecordProperty("play_reload_complete", static_cast<int>(play_totals.reload_completed));
        // 1tick(全ゲームロジック)が2ms以内であること
        EXPECT_LT(ps.p99_ns, 2'000'000LL) << "play p99 > 2ms: Game-likeが重すぎる";
        EXPECT_LT(rs.p99_ns, 2'000'000LL) << "replay p99 > 2ms: Game-likeが重すぎる";
    }
}

/**
 * @brief 同一Seedでプレイを2回実行すると全Tickのstate_hashが一致することを確認
 *
 * RNG消費位置が固定されているため，同一Seedでは必ず同一の入力列と
 * 同一のゲーム進行になることを検証する
 */
TEST(GameLikePhase0Test, SameSeedProducesSameStateHash){
    constexpr DeterministicRng::Seed SEED = 1234u;
    // 1回Tickを進めるためのラムダ式
    auto RunOnce = [&]() -> std::vector<Phase0TickSnapshot> {
        UpdatePipeline    pipeline;
        Phase0InputScript scenario{};
        pipeline.StartRun(WORLD_W, WORLD_H, SEED);
        return RunPlayPhase(pipeline, scenario, nullptr);
    };
    // 2回同一のseedでゲームプレイを実行
    const auto snaps1 = RunOnce();
    const auto snaps2 = RunOnce();

    ASSERT_EQ(snaps1.size(), snaps2.size());
    for(std::size_t i = 0; i < snaps1.size(); ++i){
        EXPECT_EQ(snaps1[i].state_hash, snaps2[i].state_hash)
            << "2回目のstate_hash不一致 tick=" << snaps1[i].tick;
    }
}

/**
 * @brief シナリオが所定の360Tickで完了することを確認
 *
 * Phase0InputScriptが正確にTotalTicks()回だけ入力を生成して
 * IsFinished()==trueになることを検証する
 */
TEST(GameLikePhase0Test, ScenarioRunsExactly360Ticks){
    Phase0InputScript scenario{};
    std::uint32_t count = 0;

    // TotalTicksと一致するか
    EXPECT_EQ(Phase0InputScript::TotalTicks(), 360u);

    while (!scenario.IsFinished()){
        // 入力生成のみ(UpdatePipelineは使わない)
        (void)scenario.GenerateNextInput();
        ++count;
    }
    EXPECT_EQ(count, 360u) << "シナリオがTotalTicks()と異なるTick数で終了した";
}

/**
 * @brief RunRecorder/RunPlaybackのラウンドトリップで入力列が保全されることを確認
 *
 * プレイ中に記録した入力をRunPlaybackで取り出すと
 * 元のInputStateと一致することを確認する軽量試験
 */
TEST(GameLikePhase0Test, RecordedInputRoundTrip){
    constexpr DeterministicRng::Seed SEED = 7u;
    constexpr std::uint32_t TICKS = 30u; // 短縮版: 30 tick だけ確認

    Phase0InputScript scenario{};
    RunRecorder       recorder;

    recorder.Start(SEED);

    std::vector<InputState> recorded_inputs;
    recorded_inputs.reserve(TICKS);
    // 30回ループして入力を記録する
    for(std::uint32_t t = 0; t < TICKS; ++t){
        const InputState input = scenario.GenerateNextInput();
        recorded_inputs.push_back(input);
        recorder.Record(input);
    }
    recorder.Stop();
    // 上記ループで記録した入力を取り出して検証する
    RunPlayback playback;
    ASSERT_EQ(
        playback.Start(recorder.GetRecord()),
        PlaybackStartResult::Started
    );

    for(std::uint32_t t = 0; t < TICKS; ++t){
        const auto opt = playback.ConsumeNextInput();
        ASSERT_TRUE(opt.has_value()) << "t=" << t;

        // RECORD_ACTION_MASKでマスクされるのでゲームプレイ用ビットのみ比較
        EXPECT_EQ(opt->GetHeldBits(),    recorded_inputs[t].GetHeldBits())
            << "held_bits 不一致 t=" << t;
        EXPECT_EQ(opt->GetPressedBits(), recorded_inputs[t].GetPressedBits())
            << "pressed_bits 不一致 t=" << t;
        EXPECT_EQ(opt->GetReleasedBits(), recorded_inputs[t].GetReleasedBits())
            << "released_bits 不一致 t=" << t;
    }
    EXPECT_TRUE(playback.IsFinished());
}

/**
 * @brief released_bitsがラウンドトリップで保全されることを手動入力で検証
 *
 * Phase0InputScriptではreleasedの発生タイミングが不明瞭なため，
 * 確実にpress → releaseが起きる手動シーケンスで専用検証する.
 *
 * 入力シーケンス:
 *   tick0: MoveRight pressed  (held=R,      pressed=R,    released=0)
 *   tick1: MoveLeft  pressed, MoveRight released
 *                             (held=L,      pressed=L,    released=R)
 *   tick2: 全リリース         (held=0,      pressed=0,    released=L)
 *
 * releasedが実際に非ゼロであることを記録前にASSERTしてから
 * RunRecorder→RunPlaybackのラウンドトリップで完全一致を確認する.
 */
TEST(GameLikePhase0Test, RecordedInput_ReleasedBits_RoundTrip){
    const std::uint32_t R = ActionBit(zimovka::Action::MoveRight);
    const std::uint32_t L = ActionBit(zimovka::Action::MoveLeft);

    const std::array<InputState, 3> hand_inputs = {{
        InputState::FromBits(R, R, 0u),     // tick0: press R
        InputState::FromBits(L, L, R),      // tick1: press L / release R
        InputState::FromBits(0u, 0u, L),    // tick2: release L
    }};

    // releasedが本当に非ゼロか事前確認(テストの前提条件の保証)
    ASSERT_EQ(hand_inputs[0].GetReleasedBits(), 0u) << "tick0: released=0 想定";
    ASSERT_EQ(hand_inputs[1].GetReleasedBits(), R)  << "tick1: MoveRight released 想定";
    ASSERT_EQ(hand_inputs[2].GetReleasedBits(), L)  << "tick2: MoveLeft  released 想定";

    // R押下→R離す，L押下→L離すを記録j
    RunRecorder recorder;
    recorder.Start(0u);
    for(const auto& inp : hand_inputs){
        recorder.Record(inp);
    }
    recorder.Stop();

    // 上記入力の記録を再現して検証
    RunPlayback playback;
    ASSERT_EQ(playback.Start(recorder.GetRecord()), PlaybackStartResult::Started);

    for(std::size_t t = 0; t < hand_inputs.size(); ++t){
        const auto opt = playback.ConsumeNextInput();
        ASSERT_TRUE(opt.has_value()) << "t=" << t;
        EXPECT_EQ(opt->GetHeldBits(),     hand_inputs[t].GetHeldBits())
            << "held_bits 不一致 t=" << t;
        EXPECT_EQ(opt->GetPressedBits(),  hand_inputs[t].GetPressedBits())
            << "pressed_bits 不一致 t=" << t;
        EXPECT_EQ(opt->GetReleasedBits(), hand_inputs[t].GetReleasedBits())
            << "released_bits 不一致 t=" << t;
    }
    EXPECT_TRUE(playback.IsFinished());
}

/**
 * @brief 3600Tickのソークテスト: Play→Replayで累積ズレを検出する
 *
 * phase0_scriptを循環させて3600tick(60秒相当)を実行し，
 * 以下の累積誤差がないことを確認する:
 *   - float演算の蓄積誤差(位置・速度)
 *   - tick_index_/next_spawn_idx_のindexズレ
 *   - reload完了タイミングの累積ズレ
 *   - RNG draw_count_の消費ズレ
 *
 * また GameLikeTotalsでshots/reloads/killsがゼロでないことを確認し，
 * 偽陰性(両方ゼロで一致)を排除する
 */
TEST(GameLikePhase0Test, SoakTest_3600Ticks){
    constexpr DeterministicRng::Seed SEED = 777u;

    // ── プレイフェーズ ──────────────────────────────────────
    UpdatePipeline   play_pipeline;
    SoakScenarioSystem scenario{};
    RunRecorder      recorder;

    recorder.Start(SEED);
    play_pipeline.StartRun(WORLD_W, WORLD_H, SEED);

    GameLikeTotals             play_totals{};
    std::vector<Phase0TickSnapshot> play_snaps =
        RunPlayPhase(play_pipeline, scenario, &recorder, &play_totals);
    recorder.Stop();

    ASSERT_EQ(play_snaps.size(), SoakScenarioSystem::TOTAL_TICKS)
        << "プレイフェーズのTick数が " << SoakScenarioSystem::TOTAL_TICKS << " でない";

    // 累積値の非ゼロ確認: 偽陰性排除
    EXPECT_GT(play_totals.shots_fired,      0u) << "3600tickで発射ゼロ(偽陰性の恐れ)";
    EXPECT_GT(play_totals.reload_started,   0u) << "3600tickでリロード開始ゼロ";
    EXPECT_GT(play_totals.reload_completed, 0u) << "3600tickでリロード完了ゼロ";
    EXPECT_GT(play_totals.enemy_kills,      0u) << "3600tickで撃破ゼロ(偽陰性の恐れ)";
    EXPECT_GT(play_totals.enemy_hits,       0u) << "3600tickでヒットゼロ(偽陰性の恐れ)";

    // ── リプレイフェーズ ────────────────────────────────────
    // 記録したシード値を取得するためのrecord
    const auto& record = recorder.GetRecord();
    // 記録したシードが違っていたら即終了
    ASSERT_EQ(record.random_seed, SEED);
    
    UpdatePipeline replay_pipeline;
    replay_pipeline.StartRun(WORLD_W, WORLD_H, record.random_seed);

    RunPlayback playback;
    ASSERT_EQ(playback.Start(recorder.GetRecord()), PlaybackStartResult::Started);

    GameLikeTotals             replay_totals{};
    std::vector<Phase0TickSnapshot> replay_snaps =
        RunReplayPhase(replay_pipeline, playback, &replay_totals);

    // ── 比較フェーズ ────────────────────────────────────────
    ASSERT_EQ(play_snaps.size(), replay_snaps.size())
        << "ソークPlay/ReplayでTick数が異なる";

    for(std::size_t i = 0; i < play_snaps.size(); ++i){
        EXPECT_EQ(play_snaps[i].state_hash, replay_snaps[i].state_hash)
            << "state_hash 不一致 tick=" << play_snaps[i].tick;
        // 最初の不一致で詳細を出して以降は省略
        if(play_snaps[i].state_hash != replay_snaps[i].state_hash) break;
    }

    // 累積値一致確認
    EXPECT_EQ(play_totals.shots_fired,      replay_totals.shots_fired)
        << "shots_fired 累積値不一致";
    EXPECT_EQ(play_totals.reload_started,   replay_totals.reload_started)
        << "reload_started 累積値不一致";
    EXPECT_EQ(play_totals.reload_completed, replay_totals.reload_completed)
        << "reload_completed 累積値不一致";
    EXPECT_EQ(play_totals.enemy_hits,       replay_totals.enemy_hits)
        << "enemy_hits 累積値不一致";
    EXPECT_EQ(play_totals.enemy_kills,      replay_totals.enemy_kills)
        << "enemy_kills 累積値不一致";
    EXPECT_EQ(play_totals.player_hits,      replay_totals.player_hits)
        << "player_hits 累積値不一致";

    // ──── タイミング統計(3600tick) ────
    {
        std::vector<std::int64_t> play_ns;
        play_ns.reserve(play_snaps.size());
        for(const auto& s : play_snaps) play_ns.push_back(s.update_ns);

        const auto ps = test_util::TimingStats::Compute(play_ns);
        RecordProperty("soak_avg_us",  ps.avg_ns / 1000);
        RecordProperty("soak_p55_us",  ps.p55_ns / 1000);
        RecordProperty("soak_p99_us",  ps.p99_ns / 1000);
        RecordProperty("soak_max_us",  ps.max_ns / 1000);
        RecordProperty("soak_shots_fired",     static_cast<int>(play_totals.shots_fired));
        RecordProperty("soak_enemy_kills",     static_cast<int>(play_totals.enemy_kills));
        RecordProperty("soak_reload_complete", static_cast<int>(play_totals.reload_completed));
        EXPECT_LT(ps.p99_ns, 2'000'000LL) << "soak p99 > 2ms: Game-likeが重すぎる";
    }
}

/**
 * @brief 同じ入力記録 + 異なるGameplayシード → 異なるゲーム状態
 *
 * gameplay_rng_のシードが異なると敵のスポーン位置・速度が変わり，
 * 衝突タイミングや残弾数が変化することを確認する.
 *
 * 手順:
 *   1. SEED_Aでプレイ&入力をRunRecorderに記録
 *   2. 記録した入力をSEED_Aで再生 → state_hash列A
 *   3. 同じ入力をSEED_Bで再生 → state_hash列B
 *   4. AとBで少なくとも1tickが異なることを確認
 *
 * NOTE: 本テストではworld_hash(RNG除外)を使い，実際のゲームワールド状態
 *       (敵位置・プレイヤー位置など)がシードの違いで異なることを検証する
 */
TEST(GameLikePhase0Test, SameInput_DifferentGameplaySeed_DifferentState){
    constexpr DeterministicRng::Seed SEED_A = 100u;
    constexpr DeterministicRng::Seed SEED_B = 200u;

    // ── プレイ + 記録 ──────────────────────────────
    UpdatePipeline    play_pipeline;
    Phase0InputScript scenario{};
    RunRecorder       recorder;

    recorder.Start(SEED_A);
    play_pipeline.StartRun(WORLD_W, WORLD_H, SEED_A);
    (void)RunPlayPhase(play_pipeline, scenario, &recorder);
    recorder.Stop();

    ASSERT_FALSE(recorder.GetRecord().frames.empty());

    // ── リプレイ A: 同じシード ──────────────────────────────
    UpdatePipeline replay_a;
    replay_a.StartRun(WORLD_W, WORLD_H, SEED_A);
    RunPlayback playback_a;
    ASSERT_EQ(playback_a.Start(recorder.GetRecord()), PlaybackStartResult::Started);
    const auto snaps_a = RunReplayPhase(replay_a, playback_a);

    // ── リプレイ B: 異なるシード(同じ入力列) ────────────────
    UpdatePipeline replay_b;
    replay_b.StartRun(WORLD_W, WORLD_H, SEED_B);
    RunPlayback playback_b;
    ASSERT_EQ(playback_b.Start(recorder.GetRecord()), PlaybackStartResult::Started);
    const auto snaps_b = RunReplayPhase(replay_b, playback_b);

    ASSERT_EQ(snaps_a.size(), snaps_b.size());

    // world_hash(RNG除外)で比較: ゲームワールドが実際に異なることを検証
    // ※state_hashはRNG値を直接含むためtrivialに常に成功してしまう
    bool any_world_diff = false;
    for(std::size_t i = 0; i < snaps_a.size(); ++i){
        if(snaps_a[i].world_hash != snaps_b[i].world_hash){
            any_world_diff = true;
            break;
        }
    }
    EXPECT_TRUE(any_world_diff)
        << "同じ入力列 + 異なるGameplaySeed(A=" << SEED_A << ", B=" << SEED_B
        << ")で全" << snaps_a.size() << "tickのworld_hashが一致"
        << " → シードがゲームワールド状態(敵位置等)に反映されていない";
    // 乱数を消費していないことを確認
    EXPECT_GT(replay_a.GetRngDrawCount(), 0u);
    EXPECT_GT(replay_b.GetRngDrawCount(), 0u);
}
