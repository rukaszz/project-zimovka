#include "zimovka/app/Application.hpp"

#include <algorithm>
#include <chrono>

#include <SDL2/SDL.h>

#include "zimovka/platform/SdlContext.hpp"
#include "zimovka/platform/Window.hpp"
#include "zimovka/rendering/Renderer.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"
#include "zimovka/input/Action.hpp"
#include "zimovka/debug/DebugOverlay.hpp"

namespace zimovka{
/**
 * @brief ゲーム実行関数
 * ループは固定デルタタイムで実現し，chronoを用いてナノ秒精度で回す
 *
 * @param argc
 * @param argv
 * @return int
 */
int Application::Run(int argc, char* argv[]){
    (void)argc;
    (void)argv;

    // SDLの初期化
    SdlContext sdl;
    Window window("Zimovka", WINDOW_WIDTH, WINDOW_HEIGHT);
    Renderer renderer(window.Get());
    PrimitiveRenderer prim(renderer.Get());
    // デバッグ情報
    // フォントのパスはとりあえずシステムフォントを使う(※環境依存なので最終的には修正する)
    DebugOverlay debug_overlay(
        renderer.Get(), 
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 
        14
    );
    // デバッグ用にナノ秒をミリ秒へ変換するラムダ式to_msの定義
    auto to_ms = [](std::chrono::nanoseconds d) -> float {
        return static_cast<float>(d.count()) / 1'000'000.0f;
    };

    // プレイヤーの初期化
    player_system_.Initialize(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));
    // 弾1200発生成
    InitializeBulletStressTest();

    // 固定タイムステップ用クロック(steady_clockはis_steadyが保証される)
    using Clock = std::chrono::steady_clock;
    // float精度の1/60
    const float fixed_delta = 1.0f / static_cast<float>(TARGET_FPS);
    // 精度を保証しつつナノ秒単位の1/60を取得
    const auto fixed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<float>(fixed_delta)
    );
    // 1/60 * 5 を更新遅延時の最大更新数とする
    const auto max_acc = fixed_ns * MAX_UPDATE_PER_FRAME;

    // ループ処理開始前の時刻取得
    // 1フレーム前のupdate開始時刻
    auto previous_update_start = Clock::now();
    // 1フレーム前のフレーム開始時刻
    auto previous_frame_start  = Clock::now();

    // 1ループ中の更新を保証する累積値
    std::chrono::nanoseconds update_time_acc{0};
    // デバッグ文字列更新用カウンタ
    auto debug_refresh_acc = Clock::duration::zero();

    running_ = true;

    while(running_){
        // ────────────────────────────────
        // フレーム時間計測(前フレームの start → 今フレームの start)
        // ────────────────────────────────
        // 現フレーム開始時刻
        auto frame_start = Clock::now();
        // 前フレーム開始時刻との差
        auto frame_elapsed = frame_start - previous_frame_start;
        previous_frame_start = frame_start;

        // 厳格にフレーム経過時刻を取得
        const float raw_frame_ms = to_ms(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                frame_elapsed
            )
        );

        // ────────────────────────────────
        // 入力受付
        // ────────────────────────────────
        ProcessEvents();

        // ────────────────────────────────
        // 更新(固定タイムステップ)
        // ────────────────────────────────
        // update処理の開始時刻
        auto update_start = Clock::now();
        // update_startと前フレームとの差
        auto update_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(update_start - previous_update_start);
        previous_update_start = update_start;
        // 1/60 * 5以上の更新遅延が生じたらclamp
        if(update_elapsed > max_acc){
            update_elapsed = max_acc;
        }
        update_time_acc += update_elapsed;
        
        // 更新回数計測用変数
        std::size_t update_steps = 0;
        
        // 0.016s単位で更新
        while(update_time_acc >= fixed_ns){
            // 入力状態のスナップショットを取得して渡す
            const InputState tick_input = input_system_.ConsumeStateForTick();
            Update(fixed_delta, tick_input);
            update_time_acc -= fixed_ns;
            ++update_steps;
        }
        // 更新処理後に時刻計測
        const float raw_update_ms = to_ms(Clock::now() - update_start);

        // ────────────────────────────────
        // ゲーム描画(デバッグOverlayは含めないことでrender_msへの影響を防ぐ)
        // ────────────────────────────────
        auto render_start = Clock::now();
        renderer.Clear();
        Render(prim);
        const float raw_render_ms = to_ms(Clock::now() - render_start);

        // ────────────────────────────────
        // デバッグOverlay(0.25s毎にflush + 描画)
        // ────────────────────────────────
        debug_refresh_acc += frame_elapsed;
        if(debug_refresh_acc > DEBUG_REFRESH_INTERVAL){
            FlushDebugStats(update_steps);
            debug_overlay.Update(debug_stats_);
            debug_refresh_acc -= DEBUG_REFRESH_INTERVAL;
        }
        debug_overlay.Render();
        // windowへ描画
        renderer.Present();

        // ────────────────────────────────
        // present後の実処理時間(CapFrameRate待機を除く)
        // ────────────────────────────────
        const float raw_processing_ms = to_ms(Clock::now() - frame_start);

        // ────────────────────────────────
        // 累積(毎フレーム)
        // ────────────────────────────────
        AccumulateDebugStats(raw_frame_ms, raw_update_ms, raw_render_ms, raw_processing_ms);

        // fpsキャップ
        CapFrameRate(frame_start);
    }

    return 0;
}

/**
 * @brief イベント処理用関数
 *
 */
void Application::ProcessEvents(){
    // SDL_Event取得
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_QUIT){
            running_ = false;
            continue;
        }
        // キー入力取得
        input_system_.HandleEvent(event);
    }
    // ESCキー入力で終了(イベント処理後に入力状態を確認し終了する)
    if(input_system_.IsPressed(zimovka::Action::Quit)){
        running_ = false;
    }
}

/**
 * @brief ゲーム更新処理
 *
 * @param dt 固定タイムステップ(秒)
 */
void Application::Update(float dt, const InputState& state){
    // 活性状態の弾がなくなったらまた弾をMAXまで表示
    if(bullet_system_.CountActive() == 0){
        InitializeBulletStressTest();
    }
    bullet_system_.Update(dt, WINDOW_WIDTH, WINDOW_HEIGHT);
    // 次にプレイヤー
    player_system_.Update(dt, state);
    // 最後に衝突判定
    if(collision_system_.CheckPlayerHitByBullets(
        player_system_.GetPlayer(), 
        bullet_system_
    ))
    {
        // 衝突した場合はプレイヤーの位置を初期化(仮)
        player_system_.Initialize(
            static_cast<float>(WINDOW_WIDTH), 
            static_cast<float>(WINDOW_HEIGHT)
        );
        bullet_system_.Clear();
    }
}

/**
 * @brief 描画処理
 *
 */
void Application::Render(PrimitiveRenderer& prim){
    bullet_system_.Render(prim);
    player_system_.Render(prim);
}

/**
 * @brief fpsキャップ
 *
 * SDL_Delayで粗く待機した後，busy waitで残りを精密に合わせる
 * → SDL_Delay単体より精度が高く，busy wait単体よりCPU負荷が低い
 *
 * @param frame_start フレーム開始時刻(steady_clock::time_point)
 */
void Application::CapFrameRate(std::chrono::steady_clock::time_point frame_start){
    // chrono 系の変数準備
    using Clock = std::chrono::steady_clock;
    using ns    = std::chrono::nanoseconds;
    using ms    = std::chrono::milliseconds;

    // 1フレームの目標時間をナノ秒で表現(整数演算なので精度劣化がない)
    constexpr ns TARGET_NS{1'000'000'000LL / TARGET_FPS};   // 16,666,666 ns

    // ナノ秒単位の経過時間(現在時刻 - ループ開始時刻)
    const ns elapsed = Clock::now() - frame_start;
    
    // ループが0.016sより早く終了することを防止するので，経過時間 < 0.016sの条件である
    if(elapsed < TARGET_NS){
        // 待機時間 = 0.016s - 経過時間
        const ns sleep_ns = TARGET_NS - elapsed;
        // SDL_Delayはミリ秒単位 → 1ms手前で止めてbusy waitへ移行
        const auto sleep_ms = std::chrono::duration_cast<ms>(sleep_ns).count();
        if(sleep_ms > 1){
            SDL_Delay(static_cast<Uint32>(sleep_ms - 1));
        }
        // 残り~1msをbusy waitで精密調整(高負荷なので1ms未満の調整のみで使う)
        while(Clock::now() - frame_start < TARGET_NS){ /* spin */ }
    }
}

// テスト用関数
/**
 * @brief 弾を瞬間的に大量に生成して負荷を試験するテスト関数
 * 
 * 呼ばれると弾を1200発生成する(1200はBulletSystemのCapacityに依る)
 */
void Application::InitializeBulletStressTest(){
    for(std::size_t i = 0; i < bullet_system_.GetCapacity(); ++i){
        // x座標を設定(24pxずつx軸へずれる)
        const float x = static_cast<float>(i % 40) * 24.0f + 12.0f;
        // y座標を設定(16pxy軸へずれる)
        const float y = static_cast<float>(i / 40) * 16.0f;
        // 弾生成
        bullet_system_.Spawn(
            Vec2{x, y}, 
            Vec2{0.0f, 60.0f}, 
            3.0f
        );
    }
}

/**
 * @brief フレーム単位のデバッグ情報の収集関数
 * 
 * @param update_steps 
 */
void Application::FlushDebugStats(const std::size_t update_steps){
    // 累積数n
    const float n = static_cast<float>(debug_acc_.count);
    // DebugStatsの各要素へ代入
    debug_stats_.frame_ms           = debug_acc_.sum_frame_ms      / n;
    debug_stats_.update_ms          = debug_acc_.sum_update_ms     / n;
    debug_stats_.render_ms          = debug_acc_.sum_render_ms     / n;
    debug_stats_.processing_ms      = debug_acc_.sum_processing_ms / n;
    debug_stats_.max_frame_ms       = debug_acc_.max_frame_ms;
    debug_stats_.max_update_ms      = debug_acc_.max_update_ms;
    debug_stats_.max_render_ms      = debug_acc_.max_render_ms;
    debug_stats_.max_processing_ms  = debug_acc_.max_processing_ms;
    debug_stats_.active_bullets     = bullet_system_.CountActive();
    debug_stats_.bullet_capacity    = bullet_system_.GetCapacity();
    debug_stats_.collision_checks   = collision_system_.LastCheckCount();
    debug_stats_.update_steps       = update_steps;
    // 最後に累積値リセット
    debug_acc_.Reset();
}

/**
 * @brief ゲームループ末尾でデバッグ情報の累積を実施する関数
 * 
 * @param raw_frame_ms 
 * @param raw_update_ms 
 * @param raw_render_ms 
 * @param raw_processing_ms 
 */
void Application::AccumulateDebugStats(
    const float raw_frame_ms, 
    const float raw_update_ms, 
    const float raw_render_ms, 
    const float raw_processing_ms
)
{
    // debug_accへデータを累積させる
    debug_acc_.sum_frame_ms      += raw_frame_ms;
    debug_acc_.sum_update_ms     += raw_update_ms;
    debug_acc_.sum_render_ms     += raw_render_ms;
    debug_acc_.sum_processing_ms += raw_processing_ms;
    debug_acc_.max_frame_ms      = std::max(debug_acc_.max_frame_ms,      raw_frame_ms);
    debug_acc_.max_update_ms     = std::max(debug_acc_.max_update_ms,     raw_update_ms);
    debug_acc_.max_render_ms     = std::max(debug_acc_.max_render_ms,     raw_render_ms);
    debug_acc_.max_processing_ms = std::max(debug_acc_.max_processing_ms, raw_processing_ms);
    ++debug_acc_.count;
}

}   // namespace zimovka
