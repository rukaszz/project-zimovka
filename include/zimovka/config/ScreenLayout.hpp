#ifndef ZIMOVKA_CONFIG_SCREENLAYOUT_HPP_
#define ZIMOVKA_CONFIG_SCREENLAYOUT_HPP_

namespace zimovka{
namespace ScreenLayout{
    
inline constexpr float LOGICAL_WIDTH  = 960.0f;      // 画面全体の幅
inline constexpr float LOGICAL_HEIGHT = 720.0f;      // 画面全体の高さ

inline constexpr float PLAYFIELD_X      = 0.0f;      // ゲームプレイ画面左端
inline constexpr float PLAYFIELD_Y      = 0.0f;      // ゲームプレイ画面上端
inline constexpr float PLAYFIELD_WIDTH  = 640.0f;    // ゲームプレイ画面幅
inline constexpr float PLAYFIELD_HEIGHT = 720.0f;    // ゲームプレイ画面高さ

inline constexpr float HUD_X      = 640.0f;   // HUD左端
inline constexpr float HUD_Y      = 0.0f;     // HUD上端
inline constexpr float HUD_WIDTH  = 320.0f;   // HUD幅
inline constexpr float HUD_HEIGHT = 720.0f;   // HUD高さ

inline constexpr float HUD_MARGIN = 24.0f;    // HUD隙間

// 画面上の定義であり広範囲に影響するので，コンパイル時にチェックする
static_assert(PLAYFIELD_X + PLAYFIELD_WIDTH == HUD_X        );
static_assert(HUD_X       + HUD_WIDTH       == LOGICAL_WIDTH);
static_assert(PLAYFIELD_Y == 0.0f);
static_assert(HUD_Y       == 0.0f);
static_assert(PLAYFIELD_HEIGHT == LOGICAL_HEIGHT);
static_assert(HUD_HEIGHT       == LOGICAL_HEIGHT);

} // namespace ScreenLayout
} // namespace zimovka

#endif  // ZIMOVKA_CONFIG_SCREENLAYOUT_HPP_
