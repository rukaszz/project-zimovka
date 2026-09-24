#include "zimovka/rendering/hud/HudRenderer.hpp"

#include "zimovka/config/ScreenLayout.hpp"
#include "zimovka/rendering/hud/HudConfig.hpp"

namespace zimovka{
/**
 * @brief HUDの描画処理
 *
 * 描画順: 背景→残弾アイコン→リロードバー(リロード中のみ)→ボムストック
 *
 * @param model      HUDに表示するデータのスナップショット
 * @param primitives プリミティブ描画ラッパ
 */
void HudRenderer::Render(
    const HudViewModel& model,
    PrimitiveRenderer& primitives
) const
{
    // 定数群の呼び出し簡略化
    using namespace ScreenLayout;
    using namespace HudConfig;

    // HUD背景
    primitives.DrawFilledRect(HUD_X, HUD_Y, HUD_WIDTH, HUD_HEIGHT, HUD_BG_COLOR);

    // 描画起点(左上マージン適用済み)
    const float hx = HUD_X + HUD_MARGIN;
    float y = HUD_Y + HUD_MARGIN;

    // 残弾アイコン
    // max_ammo個分の小矩形を横に並べ，ammo個分を明るい青で塗る
    for(std::uint32_t i = 0; i < model.max_ammo; ++i){
        // HUDの左端hxを起点として弾アイコンのx座標を設定
        const float x = hx + static_cast<float>(i) * (AMMO_ICON_W + AMMO_ICON_GAP);
        // 左少〜右多の並びで，残弾数に応じて色を分ける
        const Color c = (i < model.ammo) ? AMMO_FULL_COLOR : AMMO_EMPTY_COLOR;
        primitives.DrawFilledRect(x, y, AMMO_ICON_W, AMMO_ICON_H, c);
    }

    // HUDのアイコン描画領域のy座標をずらす
    y += AMMO_ICON_H + HUD_MARGIN;

    // リロードバー(リロード中のみ表示)
    // model.reload_progress は RenderPipeline 側で [0.0, 1.0] にclamp済み
    if(model.reloading){
        // HUD幅 - マージン2倍(左端もマージンを取っているので2倍して引かないと画面端になってしまう)
        const float bar_w = HUD_WIDTH - 2.0f * HUD_MARGIN;
        // 背景バー
        primitives.DrawFilledRect(hx, y, bar_w, RELOAD_BAR_H, RELOAD_BG_COLOR);
        // 進捗バー(1.0 = 装填完了，0.0 = 装填開始直後)
        primitives.DrawFilledRect(hx, y, bar_w * model.reload_progress, RELOAD_BAR_H, RELOAD_FILL_COLOR);
    }

    // 描画場所を下へ(リロードバー分を常に開けておく)
    y += RELOAD_BAR_H + HUD_MARGIN;

    // ボムストック
    // bomb_stock個のアイコンを横に並べる
    for(std::uint32_t i = 0; i < model.bomb_stock; ++i){
        // ボムは頻繁に増減しないので空であることを示すアイコンは描画しない
        const float x = hx + static_cast<float>(i) * (BOMB_ICON_W + BOMB_ICON_GAP);
        primitives.DrawFilledRect(x, y, BOMB_ICON_W, BOMB_ICON_H, BOMB_ICON_COLOR);
    }
}

}   // namespace zimovka
