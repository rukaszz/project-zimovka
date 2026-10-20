#include "zimovka/rendering/hud/BossHudRenderer.hpp"

#include <algorithm>

#include "zimovka/config/ScreenLayout.hpp"
#include "zimovka/rendering/Color.hpp"
#include "zimovka/rendering/hud/HudConfig.hpp"

namespace zimovka{
/**
 * @brief ボスHPバー描画
 *
 * プレイフィールド上部に横長のバーを配置する
 * 描画順序：枠 → 背景 → 残HP
 * hp_ratio > 25% : 緑  /  hp_ratio <= 25% : 赤
 *
 * @param model      描画情報のスナップショット
 * @param primitives プリミティブ描画ラッパ
 */
void BossHudRenderer::Render(
    const BossHudViewModel& model,
    PrimitiveRenderer& primitives
) const
{
    // ゲームの状態チェック
    if(!model.visible){
        return;
    }
    if(model.max_hp <= 0){
        return;
    }
    // 設定値の呼び出し簡略化
    using namespace ScreenLayout;
    using namespace HudConfig;

    // HPバーを画面上部の中央へ配置
    const float bar_x = PLAYFIELD_X + BOSS_BAR_MARGIN;
    const float bar_y = PLAYFIELD_Y + BOSS_BAR_HEIGHT + BOSS_BAR_MARGIN;
    const float bar_w = PLAYFIELD_WIDTH - 2.0f * BOSS_BAR_MARGIN;

    // HPの割合→[0.0, 1.0]
    const float hp_ratio = std::clamp(
        static_cast<float>(model.hp) / static_cast<float>(model.max_hp),
        0.0f, 1.0f
    );
    // しきい値でバーの色を切り替え
    const Color fill_color = (hp_ratio > BOSS_HP_LOW_THRESHOLD) ? BOSS_HP_HIGH_COLOR : BOSS_HP_LOW_COLOR;

    // 枠(1px大きめに描画することで輪郭を表現)
    primitives.DrawFilledRect(
        bar_x - 1.0f, bar_y - 1.0f,
        bar_w + 2.0f, BOSS_BAR_HEIGHT + 2.0f, 
        BOSS_HP_BORDER_COLOR
    );
    // 背景
    primitives.DrawFilledRect(
        bar_x, bar_y, 
        bar_w, BOSS_BAR_HEIGHT, 
        BOSS_BG_COLOR
    );
    // 残HP
    if(hp_ratio > 0.0f){
        primitives.DrawFilledRect(
            bar_x, bar_y, 
            bar_w * hp_ratio, BOSS_BAR_HEIGHT, 
            fill_color
        );
    }
}

} // namespace zimovka
