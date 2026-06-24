#include "zimovka/platform/Window.hpp"

#include <string>
#include <stdexcept>

#include <SDL2/SDL.h>

/*
 * Windowクラスは画面表示の基礎部分
 * あくまでもウィンドウの表示に関わる部分※描画はまた別
 */

/** 
 * @brief Construct a new Window:: Window object
 * 
 * @param title: タイトルを記述(SDL_CreateWindowはchar*なのでc_str()が必須)
 * @param width: windowの幅
 * @param height: windowの高さ
 */

zimovka::Window::Window(const std::string& title, int width, int height){
    // windowオブジェクト生成
    window_ = SDL_CreateWindow(
        title.c_str(), 
        SDL_WINDOWPOS_CENTERED, // ディスプレイの中央に出す
        SDL_WINDOWPOS_CENTERED, 
        width, 
        height, 
        SDL_WINDOW_SHOWN
    );
    // 生成がうまくいかなかったら終了
    if(!window_){
        throw std::runtime_error(std::string("SDL_CreateWindow failed. ") + SDL_GetError());
    }
}

/**
 * @brief Destroy the Window:: Window object
 * 
 */
zimovka::Window::~Window(){
    // Windowオブジェクト破棄
    reset();
}

/**
 * @brief windowメンバ変数破棄処理
 * 
 */
void zimovka::Window::reset() noexcept{
    // window_がnullでないなら破棄する
    if(window_){
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

/**
 * @brief ウィンドウのサイズ(描画範囲)を返却する
 * 
 * @return SDL_Point 
 */
SDL_Point zimovka::Window::getWindowSize() const{
    SDL_Point p{0, 0};
    // windowオブジェクトの存在チェック
    if(window_){
        SDL_GetWindowSize(window_, &p.x, &p.y);
    }
    return p;
}
