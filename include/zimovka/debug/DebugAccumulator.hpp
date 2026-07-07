#ifndef ZIMOVKA_DEBUG_DEBUGACCUMULATOR_HPP_
#define ZIMOVKA_DEBUG_DEBUGACCUMULATOR_HPP_

#include <algorithm>
#include <cstddef>

#include "zimovka/debug/DebugStats.hpp"

namespace zimovka{
/**
 * @brief デバッグ情報の累積値格納用構造体
 *
 * 0.25秒程度の区間でデータを累積し，FlushTo()で平均・最大値をDebugStatsへ書き出す
 */
struct DebugAccumulator{
    float       sum_frame_ms        = 0.0f;
    float       sum_update_ms       = 0.0f;
    float       sum_render_ms       = 0.0f;
    float       sum_processing_ms   = 0.0f;
    float       max_frame_ms        = 0.0f;
    float       max_update_ms       = 0.0f;
    float       max_render_ms       = 0.0f;
    float       max_processing_ms   = 0.0f;
    std::size_t max_update_steps    = 0;
    std::size_t zero_update_frames  = 0;    // update_steps == 0 のフレーム数
    std::size_t multi_update_frames = 0;    // update_steps > 1  のフレーム数
    std::size_t count               = 0;    // 累積フレーム数

    /**
     * @brief 毎フレームの計測値を累積する関数
     *
     * update_stepsの統計(max/zero/multi)もここでまとめて管理し
     * Application::Run()側への漏れを防ぐ
     *
     * @param frame_ms      フレーム間隔(ms)
     * @param update_ms     更新処理時間(ms)
     * @param render_ms     描画処理時間(ms)
     * @param processing_ms 総処理時間(ms，CapFrameRate除く)
     * @param update_steps  このフレームのUpdate実行回数
     */
    void Accumulate(
        float       frame_ms,
        float       update_ms,
        float       render_ms,
        float       processing_ms,
        std::size_t update_steps
    ) noexcept {
        sum_frame_ms      += frame_ms;
        sum_update_ms     += update_ms;
        sum_render_ms     += render_ms;
        sum_processing_ms += processing_ms;
        max_frame_ms       = std::max(max_frame_ms,      frame_ms);
        max_update_ms      = std::max(max_update_ms,     update_ms);
        max_render_ms      = std::max(max_render_ms,     render_ms);
        max_processing_ms  = std::max(max_processing_ms, processing_ms);
        max_update_steps   = std::max(max_update_steps,  update_steps);
        if(update_steps == 0){
            ++zero_update_frames;
        }
        if(update_steps > 1){
            ++multi_update_frames;
        }
        ++count;
    }

    /**
     * @brief 累積値をDebugStatsへ書き出し，自身をリセットする関数
     *
     * ゲーム固有の値(active_bullets等)はApplicationが別途設定する
     * count == 0 のときは何もしない(0除算防止)
     *
     * @param out 書き出し先のDebugStats
     */
    void FlushTo(DebugStats& out) noexcept {
        // 0除算チェック
        if(count == 0){
            return;
        }
        const float n = static_cast<float>(count);
        out.frame_ms            = sum_frame_ms      / n;
        out.update_ms           = sum_update_ms     / n;
        out.render_ms           = sum_render_ms     / n;
        out.processing_ms       = sum_processing_ms / n;
        out.max_frame_ms        = max_frame_ms;
        out.max_update_ms       = max_update_ms;
        out.max_render_ms       = max_render_ms;
        out.max_processing_ms   = max_processing_ms;
        out.max_update_steps    = max_update_steps;
        out.zero_update_frames  = zero_update_frames;
        out.multi_update_frames = multi_update_frames;
        Reset();
    }

    // リセット
    void Reset() noexcept {
        *this = {};
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_DEBUG_DEBUGACCUMULATOR_HPP_
