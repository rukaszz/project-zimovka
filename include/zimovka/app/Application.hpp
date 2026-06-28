#ifndef ZIMOVKA_APP_APPLICATION_HPP_
#define ZIMOVKA_APP_APPLICATION_HPP_

#include <chrono>

#include "zimovka/input/InputSystem.hpp"
#include "zimovka/system/player/PlayerSystem.hpp"

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
    static constexpr float FRAME_DELAY_MS = 1000.0f / static_cast<float>(TARGET_FPS);
    static constexpr int   MAX_UPDATE_PER_FRAME = 5;

    // main.cppで呼び出す
    int Run(int argc, char* argv[]);

private:
    // メインゲームループ制御
    bool running_ = true;
    // 入力
    InputSystem input_system_;
    // プレイヤーシステム
    PlayerSystem player_system_;

    // イベントの処理
    void ProcessEvents();
    // ゲームの更新
    void Update(float dt, const InputState& state);
    // 描画処理
    void Render(PrimitiveRenderer& prim);
    // fpsキャップ
    void CapFrameRate(std::chrono::steady_clock::time_point frame_start_ms);
};

}   // namespace zimovka

#endif  // ZIMOVKA_APP_APPLICATION_HPP_
