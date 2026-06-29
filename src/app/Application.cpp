#include "zimovka/app/Application.hpp"

#include <chrono>

#include <SDL2/SDL.h>

#include "zimovka/platform/SdlContext.hpp"
#include "zimovka/platform/Window.hpp"
#include "zimovka/rendering/Renderer.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"
#include "zimovka/input/Action.hpp"

/**
 * @brief ゲーム実行関数
 * ループは固定デルタタイムで実現し，chronoを用いてナノ秒精度で回す
 *
 * @param argc
 * @param argv
 * @return int
 */
int zimovka::Application::Run(int argc, char* argv[]){
    (void)argc;
    (void)argv;

    // SDLの初期化
    SdlContext sdl;
    Window window("Zimovka", WINDOW_WIDTH, WINDOW_HEIGHT);
    Renderer renderer(window.Get());
    PrimitiveRenderer prim(renderer.Get());

    // プレイヤーの初期化
    player_system_.Initialize(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));

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
    auto prev = Clock::now();
    // 1ループ中の更新を保証する累積値
    std::chrono::nanoseconds acc{0};

    running_ = true;

    while(running_){
        // フレーム開始時刻(steady_clock)
        auto frame_start = Clock::now();

        // 入力受付は最初に
        ProcessEvents();

        // 経過時間をaccに積算
        auto now = Clock::now();
        // 前回ループからの経過時間を取得
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - prev);
        prev = now;

        // 1/60 * 5以上の更新遅延が生じたらclamp
        if(elapsed > max_acc){
            elapsed = max_acc;
        }
        // 約0.016s以上経過したら更新する(accにループの経過時間を累積させる)
        acc += elapsed;
        // 0.016s毎に更新をしていく
        while(acc >= fixed_ns){
            Update(fixed_delta, input_system_.GetState());
            acc -= fixed_ns;
        }

        // 描画: clear → render → present の順を守る
        renderer.Clear();
        Render(prim);
        renderer.Present();

        // fpsキャップ
        CapFrameRate(frame_start);
    }

    return 0;
}

/**
 * @brief イベント処理用関数
 *
 */
void zimovka::Application::ProcessEvents(){
    // inputstate初期化
    input_system_.BeginFrame();
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
void zimovka::Application::Update(float dt, const InputState& state){
    player_system_.Update(dt, state);
}

/**
 * @brief 描画処理
 *
 */
void zimovka::Application::Render(PrimitiveRenderer& prim){
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
void zimovka::Application::CapFrameRate(std::chrono::steady_clock::time_point frame_start){
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
