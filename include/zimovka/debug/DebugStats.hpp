#ifndef ZIMOVKA_DEBUG_DEBUGSTATS_HPP_
#define ZIMOVKA_DEBUG_DEBUGSTATS_HPP_

#include <cstddef>
#include <cstdint>

namespace zimovka{
/**
 * @brief ゲームのデバッグに用いる情報を管理する構造体
 * 
 */
struct DebugStats{
    // 弾関係の変数
    std::size_t   active_enemy_bullets   = 0;
    std::size_t   enemy_bullet_capacity  = 0;
    std::size_t   collision_checks       = 0;
    std::size_t   active_player_bullets  = 0;
    std::size_t   player_bullet_capacity = 0;
    std::uint32_t ammo                     = 0;
    std::uint32_t max_ammo                 = 0;
    std::uint32_t cooldown_ticks_remaining = 0;
    std::uint32_t reload_ticks_remaining   = 0;
    // ゲームの情報
    // 更新遅延によるUpdate実行回数の計測用
    std::size_t max_update_steps    = 0;
    std::size_t zero_update_frames  = 0;
    std::size_t multi_update_frames = 0;
    // 平均値
    float       frame_ms            = 0.0f;   // 待機時間を含む1フレーム時間
    float       processing_ms       = 0.0f;   // 待機時間を含まない処理時間
    float       update_ms           = 0.0f;
    float       render_ms           = 0.0f;
    // 最大値
    float       max_frame_ms        = 0.0f;
    float       max_processing_ms   = 0.0f;
    float       max_update_ms       = 0.0f;
    float       max_render_ms       = 0.0f;
    // リプレイ記録
    bool        recording           = true;
};

}   // namespace zimovka

#endif  // ZIMOVKA_DEBUG_DEBUGSTATS_HPP_
