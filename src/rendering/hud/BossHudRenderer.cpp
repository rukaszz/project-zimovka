#include "zimovka/rendering/hud/BossHudRenderer.hpp"

#include <algorithm>

#include "zimovka/config/ScreenLayout.hpp"
#include "zimovka/rendering/Color.hpp"

namespace zimovka{
namespace{
// ボスHPバーのレイアウト定数(プレイフィールド下部)
constexpr float BAR_MARGIN = 8.0f;
constexpr float BAR_HEIGHT = 16.0f;

// ボスHPバーの色
constexpr Color BG_COLOR        { 30,  30,  30, 220};  // 背景(濃い灰)
constexpr Color HP_HIGH_COLOR   { 60, 200,  60, 255};  // HP > 25% : 緑
constexpr Color HP_LOW_COLOR    {220,  50,  50, 255};  // HP <= 25%: 赤
constexpr Color HP_BORDER_COLOR { 90,  90,  90, 255};  // 枠線(灰)

constexpr float HP_LOW_THRESHOLD = 0.25f;              // 低HP判定しきい値
} // anonymous namespace

/**
 * @brief ボスHPバーを描画する
 *
 * プレイフィールド下部に横長のバーを配置する。
 * 枠 → 背景 → 残HPの順で描画。
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
    if(!model.visible){ return; }
    if(model.max_hp <= 0){ return; }

    using namespace ScreenLayout;

    const float bar_x = PLAYFIELD_X + BAR_MARGIN;
    const float bar_y = PLAYFIELD_Y + PLAYFIELD_HEIGHT - BAR_HEIGHT - BAR_MARGIN;
    const float bar_w = PLAYFIELD_WIDTH - 2.0f * BAR_MARGIN;

    // HPの割合 [0.0, 1.0]
    const float hp_ratio = std::clamp(
        static_cast<float>(model.hp) / static_cast<float>(model.max_hp),
        0.0f, 1.0f
    );
    const Color fill_color = (hp_ratio > HP_LOW_THRESHOLD) ? HP_HIGH_COLOR : HP_LOW_COLOR;

    // 枠(1px大きめに描画することで輪郭を表現)
    primitives.DrawFilledRect(bar_x - 1.0f, bar_y - 1.0f,
                              bar_w + 2.0f, BAR_HEIGHT + 2.0f, HP_BORDER_COLOR);
    // 背景
    primitives.DrawFilledRect(bar_x, bar_y, bar_w, BAR_HEIGHT, BG_COLOR);
    // 残HP
    if(hp_ratio > 0.0f){
        primitives.DrawFilledRect(bar_x, bar_y, bar_w * hp_ratio, BAR_HEIGHT, fill_color);
    }
}

} // namespace zimovka
