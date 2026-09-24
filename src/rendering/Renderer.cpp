#include "zimovka/rendering/Renderer.hpp"

#include <string>
#include <stdexcept>

namespace zimovka{

/**
 * @brief Construct a new Renderer::Renderer object
 * 
 * @param window: Window&にしても良いが，RendererがWindowクラスに依存する 
 */
Renderer::Renderer(SDL_Window* window){
    // windowのnullチェック(引数をWindow&にしてもいい)
    if (!window) {
        throw std::invalid_argument("Renderer requires a valid SDL_Window.");
    }   
    // レンダラ作成(ハードウェアアクセラレーション指定)
    renderer_ = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // 作成処理チェック
    if(!renderer_){
        throw std::runtime_error(std::string("SDL_CreateRenderer failed. ") + SDL_GetError());
    }
}

/**
 * @brief Destroy the Renderer::Renderer object
 * 
 */
Renderer::~Renderer(){
    Reset();
}

/**
 * @brief 描画対象を消す関数(黒)
 * 描画処理前に全消去する際に使用する
 */
void Renderer::Clear(){
    // renderer_nullチェック
    if(renderer_){
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
    }
}

/**
 * @brief renderer_破棄処理
 * 
 */
void Renderer::Reset() noexcept{
    // renderer_がnullでなければ破棄
    if(renderer_){
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
}

/**
 * @brief レンダリングバッファを画面へ描画する
 * 
 */
void Renderer::Present(){
    // renderer_nullチェック
    if(renderer_){
        SDL_RenderPresent(renderer_);
    }
}

/**
 * @brief プレイ領域外に出た部分をクリップ(カット)する矩形を設定する
 * 
 * SDL_RenderSetClipRectで描画領域を制限する
 * 
 * @param x 
 * @param y 
 * @param w 
 * @param h 
 */
void Renderer::SetClipRect(int x, int y, int w, int h){
    // SDL_Rectへ変換
    const SDL_Rect rect{x, y, w, h};
    // 描画領域制限矩形設定(rectはポインタで渡す)
    if(SDL_RenderSetClipRect(renderer_, &rect) != 0){
        throw std::runtime_error(std::string("SDL_RenderSetClipRect failed. ") + SDL_GetError());
    }
}

/**
 * @brief 描画領域制限用clip矩形の削除処理
 * 
 * SDL_RenderSetClipRectを解除する
 */
void Renderer::ClearClipRect(){
    if(SDL_RenderSetClipRect(renderer_, nullptr) != 0){
        throw std::runtime_error(std::string("SDL_RenderSetClipRect failed. ") + SDL_GetError());
    }
}

}   // namespace zimovka
