#ifndef ZIMOVKA_RENDERING_RENDERER_HPP_
#define ZIMOVKA_RENDERING_RENDERER_HPP_

#include <SDL2/SDL.h>

namespace zimovka{

/**
 * @brief Windowと結びつく描画用抽象レイヤーRendererを管理するRAIIラッパクラス
 * 
 */
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
            Reset();                        // 既存window_の破棄
            renderer_ = other.renderer_;    // 右辺をムーブ
            other.renderer_ = nullptr;      // 右辺値は無効化
        }   
        return *this;   // ムーブされたメンバを返す
    }

    // 描画処理関数
    void Clear();
    void Reset() noexcept;
    void Present();

    // getter
    SDL_Renderer* Get() const{
        return renderer_;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_RENDERING_RENDERER_HPP_
