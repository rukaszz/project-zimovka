#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

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
// ヘルパ / シナリオ定義 (全てanonymous namespace内に格納)
// ──────────────────────────────────────────────────────────
namespace {

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
 * want_shoot     : このステップでShootを試みるか
 *                  → 実際に発射するかはRNGで確率的に決定する
 */
struct ScenarioStep {
    std::uint32_t duration_ticks;
    std::uint32_t held_bits;
    bool          want_shoot;
};

/**
 * @brief Phase0 シナリオスクリプト (合計 360 tick = 6秒)
 *
 * PlayerWeaponConfig デフォルト値:
 *   max_ammo=6, shot_cooldown_ticks=8, reload_duration_ticks=90
 *
 * 弾の速度 720 px/s, 1tick=1/60s → 12 px/tick 上昇
 * 敵出現位置: (480, 360), プレイヤー初期位置: (480, 600)
 * → 自機弾が敵に届くまで: (600-360)/12 ≈ 20 tick
 *
 * 各ステップで移動・射撃の組み合わせを網羅し，
 * 衝突/撃破/リロード/再発射が1シナリオに含まれるよう設計
 */
static constexpr std::array<ScenarioStep, 6> k_phase0_script = {{
    // ステップ0: 右移動 + 射撃 (60 tick)
    //   初弾が20tick前後で敵に命中, 6発打ち切り後にリロード開始
    { 60u, ActionBit(zimovka::Action::MoveRight), true  },
    // ステップ1: 左移動 + 射撃 (60 tick)
    //   リロード継続(90tick), 完了後に弾が再補充されて発射再開
    { 60u, ActionBit(zimovka::Action::MoveLeft),  true  },
    // ステップ2: 静止 + 射撃 (60 tick)
    { 60u, 0u,                                    true  },
    // ステップ3: 右移動 + 低速 (60 tick, 射撃なし)
    //   Shoftで低速移動, Shootなし → 弾数変化なし
    { 60u, ActionBit(zimovka::Action::MoveRight)
         | ActionBit(zimovka::Action::Slow),       false },
    // ステップ4: 左移動 + 射撃 (60 tick)
    { 60u, ActionBit(zimovka::Action::MoveLeft),  true  },
    // ステップ5: 静止 (60 tick, 射撃なし)
    { 60u, 0u,                                    false },
}};

// ── Phase0ScenarioSystem ───────────────────────────────────
/**
 * @brief Phase0 用のシナリオ入力生成クラス
 *
 * UpdatePipeline を汚染しないよう外部クラスとして定義し，
 * UpdatePipeline::UpdateTick() へ渡す InputState のみを生成する．
 *
 * ### 乱数消費の設計
 * GenerateNextInput() を呼ぶたびに DeterministicRng::UnitFloat() を
 * **必ず1回**消費する (want_shoot の値に関わらず常に消費).
 *
 *   const float roll = rng_.UnitFloat();        // 消費位置固定(毎tick1回)
 *   bool do_shoot = step.want_shoot && (roll < kShootProb);
 *
 * こうすることで，ステップ切り替えや want_shoot フラグの値が変わっても
 * 「毎tick N+k 回目の消費が行われる」という不変条件が保たれる.
 * → 同一Seedでは必ず同一の入力列が再現される.
 */
class Phase0ScenarioSystem {
public:
    // Shoot試行確率 (90%): 外部からも参照できるよう公開
    static constexpr float kShootProb = 0.90f;

    explicit Phase0ScenarioSystem(DeterministicRng::Seed seed)
        : rng_(seed)
    {}

    /**
     * @brief シードをリセットしてシナリオを最初から再生する
     * テストの「同一Seedで同一列」検証に使用する
     */
    void Reseed(DeterministicRng::Seed seed) {
        rng_.Reseed(seed);
        current_step_         = 0;
        tick_in_current_step_ = 0;
    }

    /**
     * @brief 1tick 分の InputState を生成し内部を進める
     *
     * RNG消費: want_shoot に関わらず UnitFloat() を必ず1回消費
     */
    [[nodiscard]]
    InputState GenerateNextInput() {
        const ScenarioStep& step = k_phase0_script[current_step_];

        // 毎tick1回消費 (消費位置の固定)
        const float roll     = rng_.UnitFloat();
        const bool do_shoot  = step.want_shoot && (roll < kShootProb);

        const std::uint32_t held    = step.held_bits
                                    | (do_shoot ? ActionBit(zimovka::Action::Shoot) : 0u);
        const std::uint32_t pressed = do_shoot ? ActionBit(zimovka::Action::Shoot) : 0u;

        // 内部Tickを進める
        if (++tick_in_current_step_ >= step.duration_ticks) {
            ++current_step_;
            tick_in_current_step_ = 0;
        }

        return InputState::FromBits(held, pressed, 0u);
    }

    bool IsFinished() const noexcept {
        return current_step_ >= k_phase0_script.size();
    }

    std::size_t GetCurrentStep() const noexcept {
        return current_step_;
    }

    // シナリオの総Tick数
    static constexpr std::uint32_t TotalTicks() noexcept {
        std::uint32_t total = 0;
        for (const auto& s : k_phase0_script) total += s.duration_ticks;
        return total;
    }

private:
    DeterministicRng  rng_;
    std::size_t       current_step_         = 0;
    std::uint32_t     tick_in_current_step_ = 0;
};

// ── Phase0TickSnapshot ─────────────────────────────────────
/**
 * @brief 1tick の観測可能状態スナップショット
 *
 * UpdatePipeline の公開 API から取得できるフィールドのみで構成する.
 * state_hash は全フィールドの複合ハッシュ.
 * update_ns は処理時間 (gtest では比較しない).
 */
struct Phase0TickSnapshot {
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
    // 複合ハッシュ(全観測フィールドを含む)
    std::size_t   state_hash          = 0;
    // 処理時間 [ns] ── gtest では EXPECT せず記録のみ
    std::int64_t  update_ns           = 0;
};

// ── State Hash ─────────────────────────────────────────────
/**
 * @brief 32ビット値を FNV-1a 風に混合する
 */
static std::size_t HashMix(std::size_t h, std::uint32_t v) noexcept {
    h ^= static_cast<std::size_t>(v);
    h *= 0x9e3779b97f4a7c15ULL;
    return h;
}

/**
 * @brief float を bit-cast して HashMix に渡す
 */
static std::size_t HashMixFloat(std::size_t h, float f) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &f, sizeof(bits));
    return HashMix(h, bits);
}

/**
 * @brief UpdatePipeline の観測可能状態から複合ハッシュを計算する
 *
 * 対象:
 *   - 自機弾の CountActive() と活性弾の position
 *   - 敵弾の CountActive() と活性弾の position
 *   - 武器状態 (ammo / cooldown / reload)
 *   - Tick イベント (player_hit / enemy_hit / kill / shot_fired)
 *   - 衝突判定カウント
 */
static std::size_t ComputeStateHash(
    const UpdatePipeline&     pipeline,
    const GameplayTickEvents& events
) noexcept {
    // FNV-1a 64-bit offset basis
    std::size_t h = 0xcbf29ce484222325ULL;

    // 自機弾
    const auto& pb = pipeline.GetPlayerBullets();
    h = HashMix(h, static_cast<std::uint32_t>(pb.CountActive()));
    for (const auto& b : pb.GetBullets()) {
        if (!b.active) continue;
        h = HashMixFloat(h, b.position.x);
        h = HashMixFloat(h, b.position.y);
    }

    // 敵弾 (Phase0 では常に0だが将来拡張に備えてハッシュに含める)
    const auto& eb = pipeline.GetEnemyBullets();
    h = HashMix(h, static_cast<std::uint32_t>(eb.CountActive()));
    for (const auto& b : eb.GetBullets()) {
        if (!b.active) continue;
        h = HashMixFloat(h, b.position.x);
        h = HashMixFloat(h, b.position.y);
    }

    // 武器状態
    const auto& ws = pipeline.GetPlayerWeaponSystem().GetState();
    h = HashMix(h, ws.ammo);
    h = HashMix(h, ws.cooldown_ticks_remaining);
    h = HashMix(h, ws.reload_ticks_remaining);

    // Tickイベント
    h = HashMix(h, events.player_hit           ? 1u : 0u);
    h = HashMix(h, events.weapon.shot_fired     ? 1u : 0u);
    h = HashMix(h, events.weapon.reload_started ? 1u : 0u);
    h = HashMix(h, events.weapon.reload_completed ? 1u : 0u);
    h = HashMix(h, static_cast<std::uint32_t>(events.enemy_hit.hit_count));
    h = HashMix(h, static_cast<std::uint32_t>(events.enemy_hit.kill_count));

    // 衝突判定カウント
    const auto& cs = pipeline.GetCollisionSystem().GetStats();
    h = HashMix(h, static_cast<std::uint32_t>(cs.player_vs_enemy_bullet_checks));
    h = HashMix(h, static_cast<std::uint32_t>(cs.player_bullet_vs_enemy_checks));

    return h;
}

/**
 * @brief UpdateTick() 後の状態から Phase0TickSnapshot を構築する
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
                                   pipeline.GetPlayerBullets().CountActive()),
        .enemy_bullet_count  = static_cast<std::uint32_t>(
                                   pipeline.GetEnemyBullets().CountActive()),
        .weapon_ammo         = ws.ammo,
        .weapon_cooldown     = ws.cooldown_ticks_remaining,
        .weapon_reload       = ws.reload_ticks_remaining,
        .player_hit          = events.player_hit,
        .enemy_hit_count     = static_cast<std::uint32_t>(events.enemy_hit.hit_count),
        .enemy_kill_count    = static_cast<std::uint32_t>(events.enemy_hit.kill_count),
        .shot_fired          = events.weapon.shot_fired,
        .state_hash          = ComputeStateHash(pipeline, events),
        .update_ns           = update_ns,
    };
}

// ── 実行ヘルパ ─────────────────────────────────────────────
/**
 * @brief プレイフェーズ: シナリオを1回実行してスナップショットを収集する
 *
 * @param pipeline  Initialize()済みの UpdatePipeline
 * @param scenario  Reseed 済みの Phase0ScenarioSystem
 * @param recorder  Start()済みの RunRecorder (nullptr可 → 記録しない)
 */
static std::vector<Phase0TickSnapshot> RunPlayPhase(
    UpdatePipeline&       pipeline,
    Phase0ScenarioSystem& scenario,
    RunRecorder*          recorder
) {
    std::vector<Phase0TickSnapshot> snaps;
    snaps.reserve(Phase0ScenarioSystem::TotalTicks());

    for (std::uint64_t tick = 0; !scenario.IsFinished(); ++tick) {
        const InputState input = scenario.GenerateNextInput();
        if (recorder) {
            recorder->Record(input);
        }

        const auto t0     = std::chrono::steady_clock::now();
        const auto events = pipeline.UpdateTick(DT, input);
        const auto t1     = std::chrono::steady_clock::now();

        snaps.push_back(ComputeSnapshot(
            tick, pipeline, events,
            (t1 - t0).count()
        ));
    }
    return snaps;
}

/**
 * @brief リプレイフェーズ: RunPlayback から入力を取り出してスナップショットを収集する
 *
 * @param pipeline  Initialize()済みの UpdatePipeline
 * @param playback  Start()済みの RunPlayback
 */
static std::vector<Phase0TickSnapshot> RunReplayPhase(
    UpdatePipeline& pipeline,
    RunPlayback&    playback
) {
    std::vector<Phase0TickSnapshot> snaps;

    for (std::uint64_t tick = 0; !playback.IsFinished(); ++tick) {
        const auto opt = playback.ConsumeNextInput();
        EXPECT_TRUE(opt.has_value()) << "tick=" << tick << " ConsumeNextInput()失敗";
        if (!opt.has_value()) break;

        const auto t0     = std::chrono::steady_clock::now();
        const auto events = pipeline.UpdateTick(DT, *opt);
        const auto t1     = std::chrono::steady_clock::now();

        snaps.push_back(ComputeSnapshot(
            tick, pipeline, events,
            (t1 - t0).count()
        ));
    }
    return snaps;
}

} // anonymous namespace

// ──────────────────────────────────────────────────────────
// Phase0 Game-like 複合試験
// ──────────────────────────────────────────────────────────
/**
 * @brief プレイ→リプレイで全Tickのstate_hashが一致することを確認
 *
 * 手順:
 *   1. Phase0ScenarioSystem(seed)で入力列を生成しながらプレイを実行
 *      RunRecorderで入力を記録，各Tickのスナップショットを収集
 *   2. RunPlaybackで記録を再生しながらリプレイを実行，スナップショットを収集
 *   3. 全Tickのstate_hashと個別フィールドが一致することを確認
 *
 * 処理時間(update_ns)はスナップショットに記録するが EXPECT_* では比較しない
 */
TEST(GameLikePhase0Test, PlayAndReplayProduceSameStateHash){
    constexpr DeterministicRng::Seed SEED = 42u;

    // ── プレイフェーズ ──────────────────────────────────────
    UpdatePipeline       play_pipeline;
    Phase0ScenarioSystem scenario(SEED);
    RunRecorder          recorder;

    play_pipeline.Initialize(WORLD_W, WORLD_H);
    recorder.Start(SEED);

    const auto play_snaps = RunPlayPhase(play_pipeline, scenario, &recorder);

    recorder.Stop();
    ASSERT_FALSE(recorder.GetRecord().frames.empty())
        << "RunRecorderが空: 入力が記録されていない";
    ASSERT_EQ(recorder.GetRecord().frames.size(), play_snaps.size())
        << "記録フレーム数とスナップショット数が不一致";

    // ── リプレイフェーズ ────────────────────────────────────
    UpdatePipeline replay_pipeline;
    replay_pipeline.Initialize(WORLD_W, WORLD_H);

    RunPlayback playback;
    ASSERT_EQ(
        playback.Start(recorder.GetRecord()),
        PlaybackStartResult::Started
    ) << "RunPlayback::Start() が失敗";

    const auto replay_snaps = RunReplayPhase(replay_pipeline, playback);

    // ── 比較フェーズ ────────────────────────────────────────
    ASSERT_EQ(play_snaps.size(), replay_snaps.size())
        << "プレイとリプレイでTick数が異なる";

    for (std::size_t i = 0; i < play_snaps.size(); ++i) {
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

    // 処理時間は比較せず合計値として記録のみ (参考値)
    std::int64_t play_total_ns   = 0;
    std::int64_t replay_total_ns = 0;
    for (const auto& s : play_snaps)   play_total_ns   += s.update_ns;
    for (const auto& s : replay_snaps) replay_total_ns += s.update_ns;

    // NOTE: RecordProperty はgtest XML出力に付加される参考値
    RecordProperty("play_total_us",   static_cast<int>(play_total_ns   / 1000));
    RecordProperty("replay_total_us", static_cast<int>(replay_total_ns / 1000));
}

/**
 * @brief 同一Seedでプレイを2回実行すると全Tickのstate_hashが一致することを確認
 *
 * RNG消費位置が固定されているため，同一Seedでは必ず同一の入力列と
 * 同一のゲーム進行になることを検証する
 */
TEST(GameLikePhase0Test, SameSeedProducesSameStateHash){
    constexpr DeterministicRng::Seed SEED = 1234u;

    auto RunOnce = [&]() -> std::vector<Phase0TickSnapshot> {
        UpdatePipeline       pipeline;
        Phase0ScenarioSystem scenario(SEED);
        pipeline.Initialize(WORLD_W, WORLD_H);
        return RunPlayPhase(pipeline, scenario, nullptr);
    };

    const auto snaps1 = RunOnce();
    const auto snaps2 = RunOnce();

    ASSERT_EQ(snaps1.size(), snaps2.size());
    for (std::size_t i = 0; i < snaps1.size(); ++i) {
        EXPECT_EQ(snaps1[i].state_hash, snaps2[i].state_hash)
            << "2回目のstate_hash不一致 tick=" << snaps1[i].tick;
    }
}

/**
 * @brief 異なるSeedでは少なくとも一部のTickのstate_hashが異なることを確認
 *
 * RNGのShoot確率(90%)により射撃タイミングが変わり，自機弾の軌跡や
 * 衝突タイミングが変化するため，異なるSeedでは状態が分岐する
 */
TEST(GameLikePhase0Test, DifferentSeedsDifferentScenario){
    auto RunWithSeed = [](DeterministicRng::Seed seed) -> std::vector<Phase0TickSnapshot> {
        UpdatePipeline       pipeline;
        Phase0ScenarioSystem scenario(seed);
        pipeline.Initialize(WORLD_W, WORLD_H);
        return RunPlayPhase(pipeline, scenario, nullptr);
    };

    const auto snaps_a = RunWithSeed(0u);
    const auto snaps_b = RunWithSeed(99999u);

    ASSERT_EQ(snaps_a.size(), snaps_b.size());

    bool any_diff = false;
    for (std::size_t i = 0; i < snaps_a.size(); ++i) {
        if (snaps_a[i].state_hash != snaps_b[i].state_hash) {
            any_diff = true;
            break;
        }
    }
    EXPECT_TRUE(any_diff)
        << "Seed=0 と Seed=99999 で全 " << snaps_a.size()
        << " tick のstate_hashが一致(確率的に不一致が期待される)";
}

/**
 * @brief シナリオが所定の360 tick で完了することを確認
 *
 * Phase0ScenarioSystemが正確に TotalTicks() 回だけ入力を生成して
 * IsFinished()==trueになることを検証する
 */
TEST(GameLikePhase0Test, ScenarioRunsExactly360Ticks){
    Phase0ScenarioSystem scenario(0u);
    std::uint32_t count = 0;

    // TotalTicksと一致するか
    EXPECT_EQ(Phase0ScenarioSystem::TotalTicks(), 360u);

    while (!scenario.IsFinished()) {
        // 入力生成のみ(UpdatePipelineは使わない)
        (void)scenario.GenerateNextInput();
        ++count;
    }
    EXPECT_EQ(count, 360u) << "シナリオがTotalTicks()と異なるTick数で終了した";
}

/**
 * @brief RunRecorder/RunPlaybackのラウンドトリップで入力列が保全されることを確認
 *
 * プレイ中に記録した入力を RunPlayback で取り出すと
 * 元の InputState と一致することを確認する軽量試験
 */
TEST(GameLikePhase0Test, RecordedInputRoundTrip){
    constexpr DeterministicRng::Seed SEED = 7u;
    constexpr std::uint32_t TICKS = 30u; // 短縮版: 30 tick だけ確認

    Phase0ScenarioSystem scenario(SEED);
    RunRecorder          recorder;
    recorder.Start(SEED);

    std::vector<InputState> recorded_inputs;
    recorded_inputs.reserve(TICKS);

    for (std::uint32_t t = 0; t < TICKS; ++t) {
        const InputState input = scenario.GenerateNextInput();
        recorded_inputs.push_back(input);
        recorder.Record(input);
    }
    recorder.Stop();

    RunPlayback playback;
    ASSERT_EQ(
        playback.Start(recorder.GetRecord()),
        PlaybackStartResult::Started
    );

    for (std::uint32_t t = 0; t < TICKS; ++t) {
        const auto opt = playback.ConsumeNextInput();
        ASSERT_TRUE(opt.has_value()) << "t=" << t;

        // RECORD_ACTION_MASKでマスクされるのでゲームプレイ用ビットのみ比較
        EXPECT_EQ(opt->GetHeldBits(),    recorded_inputs[t].GetHeldBits())
            << "held_bits 不一致 t=" << t;
        EXPECT_EQ(opt->GetPressedBits(), recorded_inputs[t].GetPressedBits())
            << "pressed_bits 不一致 t=" << t;
    }
    EXPECT_TRUE(playback.IsFinished());
}
