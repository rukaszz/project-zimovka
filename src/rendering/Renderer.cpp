#include "zimovka/rendering/Renderer.hpp"

#include <string>
#include <stdexcept>

/**
 * @brief Construct a new zimovka::Renderer::Renderer object
 * 
 * @param window: Window&にしても良いが，RendererがWindowクラスに依存する 
 */
zimovka::Renderer::Renderer(SDL_Window* window){
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
 * @brief Destroy the zimovka::Renderer::Renderer object
 * 
 */
zimovka::Renderer::~Renderer(){
    Reset();
}

/**
 * @brief 描画対象を消す関数(黒)
 * 描画処理前に全消去する際に使用する
 */
void zimovka::Renderer::Clear(){
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
void zimovka::Renderer::Reset() noexcept{
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
void zimovka::Renderer::Present(){
    // renderer_nullチェック
    if(renderer_){
        SDL_RenderPresent(renderer_);
    }
}
