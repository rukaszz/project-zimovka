#ifndef ZIMOVKA_DEBUG_DEBUGSTATUS_HPP_
#define ZIMOVKA_DEBUG_DEBUGSTATUS_HPP_

#include <cstddef>

namespace zimovka{
/**
 * @brief ゲームのデバッグに用いる情報を管理する構造体
 * 
 */
struct DebugStatus{
    // 弾関係の変数
    std::size_t active_bullets = 0;
    std::size_t bullet_capacity = 0;
    std::size_t collision_checks = 0;
    // ゲームの情報
    float frame_ms = 0.0f;
    float update_ms = 0.0f;
    float render_ms = 0.0f;
};

}   // namespace zimovka

#endif  // ZIMOVKA_DEBUG_DEBUGSTATUS_HPP_
