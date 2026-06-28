#ifndef ZIMOVKA_RENDERING_PRIMITIVERENDERER_HPP_
#define ZIMOVKA_RENDERING_PRIMITIVERENDERER_HPP_

#include <SDL2/SDL.h>

namespace zimovka{

/**
 * @brief SDL2プリミティブ描画ラッパー
 *
 * スプライト実装前の開発段階でゲームオブジェクトを矩形・円で可視化する．
 * 生ポインタでSDL_Renderer*を受け取るので所有権は持たない(ダングリングポインタにならないように注意)．
 * 座標はfloat(ゲーム座標系)を受け取り，内部でintへキャストする．
 *
 */
class PrimitiveRenderer{
public:
    explicit PrimitiveRenderer(SDL_Renderer* renderer);

    // 塗りつぶし矩形（プレイヤー・敵）
    void DrawFilledRect(float x, float y, float w, float h, SDL_Color color);
    // ワイヤーフレーム矩形（ヒットボックス確認用）
    void DrawRect(float x, float y, float w, float h, SDL_Color color);
    // 塗りつぶし円（弾丸）
    void DrawFilledCircle(float cx, float cy, int radius, SDL_Color color);

private:
    SDL_Renderer* renderer_;
    void SetColor(SDL_Color color);
};

}   // namespace zimovka

#endif  // ZIMOVKA_RENDERING_PRIMITIVERENDERER_HPP_
