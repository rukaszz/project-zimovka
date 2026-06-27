#ifndef ZIMOVKA_PLATFORM_WINDOW_HPP_
#define ZIMOVKA_PLATFORM_WINDOW_HPP_

#include <string>

#include <SDL2/SDL.h>

namespace zimovka{

/**
 * @brief SDLのウィンドウシステムのRAII管理用ラッパクラス
 * 
 */
class Window{
private:
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
        // 異なるwindow_への=演算子によるムーブ
        if(this != &other){
            Reset();                    // 既存window_の破棄
            window_ = other.window_;    // 右辺をムーブ
            other.window_ = nullptr;    // 右辺値は無効化
        }
        return *this;   // ムーブされたメンバを返す
    }

    // window片付け処理
    void Reset() noexcept;
    // windowサイズ返却
    SDL_Point GetWindowSize() const;

    // windowオブジェクト取得用
    SDL_Window* Get() const{
        return window_;
    }
};

}   // namespace zimovka

#endif // ZIMOVKA_PLATFORM_WINDOW_HPP_
