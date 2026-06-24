#ifndef RENDERING_RENDERER_HPP_
#define RENDERING_RENDERER_HPP_

#include <SDL2/SDL.h>

namespace zimovka{

class Renderer{
private:
    // SDL_Renderer*オブジェクト
    SDL_Renderer* renderer_ = nullptr;
public:
    // コンストラクタ・デストラクタ
    Renderer(SDL_Window* window);
    ~Renderer();
    // コピー禁止
    Renderer(const Renderer&) = delete; // コピーコンストラクタ禁止
    Renderer& operator=(const Renderer&) = delete;  // コピー代入禁止
    // ムーブ
    // ムーブコンストラクタ
    Renderer(Renderer&& other) noexcept : renderer_(other.renderer_){
        other.renderer_ = nullptr;    // 所有権が奪われたので無効化
    }
    // ムーブ代入演算子
    Renderer& operator=(Renderer&& other) noexcept{
        // 異なるrenderer_への=演算子によるムーブ
        if(this != &other){
            reset();                        // 既存window_の破棄
            renderer_ = other.renderer_;    // 右辺をムーブ
            other.renderer_ = nullptr;      // 右辺値は無効化
        }   
        return *this;   // ムーブされたメンバを返す
    }

    // 描画処理関数
    void clear();
    void reset() noexcept;
    void present();

    // 簡易的な動作確認用関数
    void setDrawColoer();
    // getter
    SDL_Renderer* get() const{
        return renderer_;
    }
};

}   // namespace zimovka

#endif  // RENDERING_RENDERER_HPP_
