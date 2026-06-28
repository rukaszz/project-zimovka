#include "zimovka/rendering/PrimitiveRenderer.hpp"

#include <cmath>
#include <stdexcept>

// このファイルのみで使用するユーティリティ関数
namespace {
/**
 * @brief float->intのキャストで四捨五入する
 * ※単なるキャストでは4.99fも4に落ちるため
 * 
 * @param v 
 * @return int 
 */
int ToScreenInt(float v){
    return static_cast<int>(std::lround(v));
}
} // namespace

/**
 * @brief Construct a new zimovka::Primitive Renderer::Primitive Renderer object
 * 
 * @param renderer 生ポインタでrendererを参照する
 */
zimovka::PrimitiveRenderer::PrimitiveRenderer(SDL_Renderer* renderer)
    : renderer_(renderer)
{
    if (!renderer_) {
        throw std::invalid_argument("PrimitiveRenderer requires a valid SDL_Renderer.");
    }
}

/**
 * @brief SDL_SetRenderDrawColorのラッパ
 * SDL_SetRenderDrawColorとは
 * Rendererの描画色を設定する関数
 * SDL_RenderClear()や矩形などの描画色の指定に使われる
 * 
 * @param c 
 */
void zimovka::PrimitiveRenderer::SetColor(SDL_Color c){
    SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, c.a);
}

/**
 * @brief 塗りつぶし矩形描画
 *
 * @param x 左上X座標(ゲーム座標)
 * @param y 左上Y座標(ゲーム座標)
 * @param w 幅
 * @param h 高さ
 * @param color 描画色
 */
void zimovka::PrimitiveRenderer::DrawFilledRect(float x, float y, float w, float h, SDL_Color color){
    // 色設定
    SetColor(color);
    // SDL_Rectはint型なのでキャスト
    const SDL_Rect rect{
        ToScreenInt(x),
        ToScreenInt(y),
        ToScreenInt(w),
        ToScreenInt(h)
    };
    // 矩形描画
    SDL_RenderFillRect(renderer_, &rect);
}

/**
 * @brief 塗りつぶしなし矩形描画
 *
 * @param x 左上X座標(ゲーム座標)
 * @param y 左上Y座標(ゲーム座標)
 * @param w 幅
 * @param h 高さ
 * @param color 描画色
 */
void zimovka::PrimitiveRenderer::DrawRect(float x, float y, float w, float h, SDL_Color color){
    // 色設定
    SetColor(color);
    // SDL_Rectはint型なのでキャスト
    const SDL_Rect rect{
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(w),
        static_cast<int>(h)
    };
    // 矩形描画
    SDL_RenderDrawRect(renderer_, &rect);
}

/**
 * @brief 塗りつぶし円描画
 *
 * 各y行についてsqrt(r^2 - y^2)から横幅を求め，
 * 水平線を描いて円を近似する
 * 開発用の簡易描画であり，大量弾の本番描画には使わない
 *
 * @param cx 中心X座標(ゲーム座標)
 * @param cy 中心Y座標(ゲーム座標)
 * @param radius 半径(ピクセル)
 * @param color 描画色
 */
void zimovka::PrimitiveRenderer::DrawFilledCircle(float cx, float cy, int radius, SDL_Color color){
    if(radius <= 0){
        return;
    }
    // 色設定
    SetColor(color);
    // intキャスト
    const int icx = static_cast<int>(cx);
    const int icy = static_cast<int>(cy);
    const int r2  = radius * radius;
    // y座標の最小値(-r)からy座標の最大値(r)まで増加させる
    for(int dy = -radius; dy <= radius; ++dy){
        // 0〜r2〜0のように増加するx座標
        const int dx = static_cast<int>(std::sqrt(static_cast<float>(r2 - dy * dy)));
        // 中心(icx, icy)から±dx，±dyの線分を円の下から上へ(積み上げて)塗りつぶすように描いて，円を近似する
        SDL_RenderDrawLine(renderer_, icx - dx, icy + dy, icx + dx, icy + dy);
    }
}
