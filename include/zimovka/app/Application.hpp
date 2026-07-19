#ifndef ZIMOVKA_APP_APPLICATION_HPP_
#define ZIMOVKA_APP_APPLICATION_HPP_

#include <chrono>
#include <cstddef>

#include "zimovka/input/InputSystem.hpp"
#include "zimovka/engine/update/UpdatePipeline.hpp"
#include "zimovka/replay/RunRecorder.hpp"

#include "zimovka/debug/DebugStats.hpp"
#include "zimovka/debug/DebugAccumulator.hpp"

namespace zimovka{

// 前方宣言
class PrimitiveRenderer;

/**
 * @brief ゲームのメインループを管理するクラス
 * 
 */
class Application{
public:
    // ターゲットFPSと1フレームの目標時間(ms)
    static constexpr int   TARGET_FPS     = 60;
    static constexpr int   MAX_UPDATE_PER_FRAME = 5;
    // 画面サイズ
    static constexpr int   WINDOW_WIDTH = 960;
    static constexpr int   WINDOW_HEIGHT = 720;
    // 固定ランダムシード
    static constexpr std::uint32_t INITIAL_SEED = 0x12345678u;
    // デバッグ情報の更新間隔(0.25sで描画更新)
    static constexpr auto  DEBUG_REFRESH_INTERVAL = std::chrono::milliseconds{250};

    // main.cppで呼び出す
    int Run(int argc, char* argv[]);

private:
    // メインゲームループ制御
    bool running_ = true;
    // 更新順序管理(サブシステムを所有する)
    UpdatePipeline update_pipeline_;
    // 入力
    InputSystem input_system_;
    // 入力記録
    RunRecorder run_recorder_;
    // デバッグ情報
    DebugStats debug_stats_;
    // デバッグ統計の累積(0.25s毎にflushして平均・最大値を計算)
    DebugAccumulator debug_acc_;

    // イベントの処理
    void ProcessEvents();
    // ゲームの更新
    void Update(float dt, const InputState& input);
    // 描画処理
    void Render(PrimitiveRenderer& prim);
    // fpsキャップ
    void CapFrameRate(std::chrono::steady_clock::time_point frame_start_ms);
    // デバッグ情報をdebug_acc_からdebug_stats_へ書き出す
    void FlushDebugStats();

};

}   // namespace zimovka

#endif  // ZIMOVKA_APP_APPLICATION_HPP_
