#ifndef ZIMOVKA_APP_APPLICATION_HPP_
#define ZIMOVKA_APP_APPLICATION_HPP_

#include <chrono>

#include "zimovka/input/InputSystem.hpp"
#include "zimovka/systems/player/PlayerSystem.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/collision/CollisionSystem.hpp"
#include "zimovka/debug/DebugStats.hpp"

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
    // デバッグ情報の更新間隔
    static constexpr float DEBUG_REFRESH_INTERVAL = 0.25f;

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
    struct DebugAccumulator{
        float       sum_frame_ms      = 0.0f;
        float       sum_update_ms     = 0.0f;
        float       sum_render_ms     = 0.0f;
        float       sum_processing_ms = 0.0f;
        float       max_frame_ms      = 0.0f;
        float       max_update_ms     = 0.0f;
        float       max_render_ms     = 0.0f;
        float       max_processing_ms = 0.0f;
        std::size_t count             = 0;
        void Reset() noexcept {
            *this = {};
    }
    };
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

};

}   // namespace zimovka

#endif  // ZIMOVKA_APP_APPLICATION_HPP_
