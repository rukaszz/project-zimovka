#ifndef ZIMOVKA_APP_APPLICATION_HPP_
#define ZIMOVKA_APP_APPLICATION_HPP_

#include <chrono>
#include <cstddef>

#include "zimovka/input/InputSystem.hpp"
#include "zimovka/systems/player/PlayerSystem.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/collision/CollisionSystem.hpp"
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
    static constexpr float FRAME_DELAY_MS = 1000.0f / static_cast<float>(TARGET_FPS);
    static constexpr int   MAX_UPDATE_PER_FRAME = 5;
    // 画面サイズ
    static constexpr int WINDOW_WIDTH = 960;
    static constexpr int WINDOW_HEIGHT = 720;
    // デバッグ情報の更新間隔(0.25sで描画更新)
    static constexpr auto DEBUG_REFRESH_INTERVAL = std::chrono::milliseconds{250};

    // main.cppで呼び出す
    int Run(int argc, char* argv[]);

private:
    // メインゲームループ制御
    bool running_ = true;
    // 入力
    InputSystem input_system_;
    // プレイヤーシステム
    PlayerSystem player_system_;
    // バレットシステム
    BulletSystem bullet_system_;    // デフォルトプールは1200
    float bullet_spawn_timer_ = 0.0f;
    Vec2 temp_spawn_{7.0f, 11.0f};  // スポーン用の変数(一時的なもの)
    // 衝突システム
    CollisionSystem collision_system_;
    // デバッグ情報
    DebugStats debug_stats_;
    // デバッグ統計の累積(0.25s毎にflushして平均・最大値を計算)
    DebugAccumulator debug_acc_;

    // イベントの処理
    void ProcessEvents();
    // ゲームの更新
    void Update(float dt, const InputState& state);
    // 描画処理
    void Render(PrimitiveRenderer& prim);
    // fpsキャップ
    void CapFrameRate(std::chrono::steady_clock::time_point frame_start_ms);

    // 性能試験用関数
    // 弾を瞬間的に
    void InitializeBulletStressTest();
    // デバッグ情報の累積
    void AccumulateDebugStats(
        const float raw_frame_ms, 
        const float raw_update_ms, 
        const float raw_render_ms, 
        const float raw_processing_ms 
    );
    // デバッグ情報描画用のdebug_stats_へ渡す
    void FlushDebugStats(const std::size_t update_steps);

};

}   // namespace zimovka

#endif  // ZIMOVKA_APP_APPLICATION_HPP_
