#ifndef ZIMOVKA_DEBUG_DEBUGACCUMULATOR_HPP_
#define ZIMOVKA_DEBUG_DEBUGACCUMULATOR_HPP_

#include <cstddef>

namespace zimovka{
/**
 * @brief デバッグ情報の累積値格納用構造体
 * 
 * 0.25秒くらいで更新して平均・最大値の計算に用いる
 */
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
    // リセット用メソッド
    void Reset() noexcept {
        *this = {};
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_DEBUG_DEBUGACCUMULATOR_HPP_
