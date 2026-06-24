#include "zimovka/rendering/Renderer.hpp"

#include <string>
#include <stdexcept>

/**
 * @brief Construct a new zimovka::Renderer::Renderer object
 * 
 * @param window 
 */
zimovka::Renderer::Renderer(SDL_Window* window){
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
    reset();
}

/**
 * @brief 描画対象を消す関数(黒)
 * 描画処理前に全消去する際に使用する
 */
void zimovka::Renderer::clear(){
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
}

/**
 * @brief renderer_破棄処理
 * 
 */
void zimovka::Renderer::reset() noexcept{
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
void zimovka::Renderer::present(){
    SDL_RenderPresent(renderer_);
}

/**
 * @brief テスト用の描画処理
 * 
 */
void zimovka::Renderer::setDrawColoer(){
    // 紫
    SDL_SetRenderDrawColor(renderer_, 128, 0, 128, 255);
}
