#ifndef ZIMOVKA_RENDERING_HUD_HUDCONFIG_HPP_
#define ZIMOVKA_RENDERING_HUD_HUDCONFIG_HPP_

#include "zimovka/rendering/Color.hpp"

/**
 * @brief HUDに関係する設定定数群
 * 
 */
namespace zimovka{
namespace HudConfig{
    
// 残弾アイコンのサイズと間隔
inline constexpr float AMMO_ICON_W   = 16.0f;
inline constexpr float AMMO_ICON_H   = 24.0f;
inline constexpr float AMMO_ICON_GAP =  6.0f;

// リロードバーの高さ
inline constexpr float RELOAD_BAR_H  = 10.0f;

// ボムアイコンのサイズと間隔
inline constexpr float BOMB_ICON_W   = 24.0f;
inline constexpr float BOMB_ICON_H   = 24.0f;
inline constexpr float BOMB_ICON_GAP =  8.0f;

// 各パーツのカラー
inline constexpr Color HUD_BG_COLOR      { 20,  20,  20, 200};  // HUD背景(灰)
inline constexpr Color AMMO_FULL_COLOR   {120, 220, 255, 255};  // 残弾あり(青)
inline constexpr Color AMMO_EMPTY_COLOR  { 60,  80,  90, 255};  // 弾切れ(暗い青)
inline constexpr Color RELOAD_BG_COLOR   { 60,  60,  60, 255};  // リロードバー背景(灰)
inline constexpr Color RELOAD_FILL_COLOR {100, 200, 100, 255};  // リロード進捗(緑)
inline constexpr Color BOMB_ICON_COLOR   {200, 100,  50, 255};  // ボムアイコン(赤)

} // namespace HudConfig
} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_HUD_HUDCONFIG_HPP_
