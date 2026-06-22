#ifndef PLATFORM_WINDOW_H_
#define PLATFORM_WINDOW_H_

#include <string>

#include <SDL2/SDL.h>

/**
 * @brief SDLのウィンドウシステムのRAII管理用ラッパクラス
 * 
 */
class Window{
private:
    // SDL_Window変数
    SDL_Window* window_ = nullptr;

public:
    // コンストラクタ・デストラクタ
    Window(const std::string& title, int width, int height);
    ~Window();
    
    // コピー禁止
    Window(const Window&) = delete; // コピーコンストラクタ禁止
    Window& operator=(const Window&) = delete;  // コピー代入禁止
    
    // ムーブコンストラクタ
    Window(Window&& other) noexcept : window_(other.window_){
        other.window_ = nullptr;    // 所有権が奪われたので無効化
    }
    // ムーブ代入演算子
    Window& operator=(Window&& other) noexcept{
        if(this != &other){
            window_ = other.window_;    // 右辺をムーブ
            other.window_ = nullptr;    // 右辺値は無効化
        }
        return *this;   // ムーブされたメンバを返す
    }

    // windowサイズ返却
    SDL_Point getWindowSize() const;

    // windowオブジェクト取得用
    SDL_Window* get() const{
        return window_;
    }
};

#endif // PLATFORM_WINDOW_H_
