#include "zimovka/rendering/hud/HudRenderer.hpp"

#include "zimovka/config/ScreenLayout.hpp"
#include "zimovka/rendering/Color.hpp"

namespace zimovka{

namespace{
    // 残弾アイコンのサイズと間隔
    constexpr float AMMO_ICON_W   = 16.0f;
    constexpr float AMMO_ICON_H   = 24.0f;
    constexpr float AMMO_ICON_GAP =  6.0f;

    // リロードバーの高さ
    constexpr float RELOAD_BAR_H  = 10.0f;

    // ボムアイコンのサイズと間隔
    constexpr float BOMB_ICON_W   = 24.0f;
    constexpr float BOMB_ICON_H   = 24.0f;
    constexpr float BOMB_ICON_GAP =  8.0f;

    // 各パーツのカラー
    constexpr Color HUD_BG_COLOR      { 20,  20,  20, 200};  // HUD背景
    constexpr Color AMMO_FULL_COLOR   {120, 220, 255, 255};  // 残弾あり
    constexpr Color AMMO_EMPTY_COLOR  { 60,  80,  90, 255};  // 弾切れ
    constexpr Color RELOAD_BG_COLOR   { 60,  60,  60, 255};  // リロードバー背景
    constexpr Color RELOAD_FILL_COLOR {100, 200, 100, 255};  // リロード進捗
    constexpr Color BOMB_ICON_COLOR   {200, 100,  50, 255};  // ボムアイコン
}   // anonymous namespace

/**
 * @brief HUDの描画処理
 *
 * 描画順: 背景 → 残弾アイコン → リロードバー(リロード中のみ) → ボムストック
 *
 * @param model      HUDに表示するデータのスナップショット
 * @param primitives プリミティブ描画ラッパ
 */
void HudRenderer::Render(
    const HudViewModel& model,
    PrimitiveRenderer& primitives
) const
{
    using namespace ScreenLayout;

    // HUD背景
    primitives.DrawFilledRect(HUD_X, HUD_Y, HUD_WIDTH, HUD_HEIGHT, HUD_BG_COLOR);

    // 描画起点(左上マージン適用済み)
    const float hx = HUD_X + HUD_MARGIN;
    float y = HUD_Y + HUD_MARGIN;

    // ── 残弾アイコン ──────────────────────────────────────
    // max_ammo 個の小矩形を横に並べ，ammo 個分を明るく塗る
    for(std::uint32_t i = 0; i < model.max_ammo; ++i){
        const float x = hx + static_cast<float>(i) * (AMMO_ICON_W + AMMO_ICON_GAP);
        const Color c = (i < model.ammo) ? AMMO_FULL_COLOR : AMMO_EMPTY_COLOR;
        primitives.DrawFilledRect(x, y, AMMO_ICON_W, AMMO_ICON_H, c);
    }
    y += AMMO_ICON_H + HUD_MARGIN;

    // ── リロードバー (リロード中のみ表示) ────────────────────
    if(model.reloading && model.reload_duration_ticks > 0){
        const float bar_w = HUD_WIDTH - 2.0f * HUD_MARGIN;
        const float progress = 1.0f
            - static_cast<float>(model.reload_ticks_remaining)
            / static_cast<float>(model.reload_duration_ticks);
        // 背景バー
        primitives.DrawFilledRect(hx, y, bar_w, RELOAD_BAR_H, RELOAD_BG_COLOR);
        // 進捗バー(1.0 = 装填完了，0.0 = 装填開始直後)
        primitives.DrawFilledRect(hx, y, bar_w * progress, RELOAD_BAR_H, RELOAD_FILL_COLOR);
        y += RELOAD_BAR_H + HUD_MARGIN;
    }

    // ── ボムストック ──────────────────────────────────────
    // bomb_stock 個のアイコンを横に並べる
    for(std::uint32_t i = 0; i < model.bomb_stock; ++i){
        const float x = hx + static_cast<float>(i) * (BOMB_ICON_W + BOMB_ICON_GAP);
        primitives.DrawFilledRect(x, y, BOMB_ICON_W, BOMB_ICON_H, BOMB_ICON_COLOR);
    }
}

}   // namespace zimovka
