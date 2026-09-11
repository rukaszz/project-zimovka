#ifndef ZIMOVKA_RENDERING_TEXTURE_HPP_
#define ZIMOVKA_RENDERING_TEXTURE_HPP_

#include <filesystem>
#include <utility>

#include <SDL2/SDL.h>

namespace zimovka{
/**
 * @brief SDL_TextureのRAIIラッパクラス
 * 
 */
class Texture{
private:
    SDL_Texture* texture_ = nullptr;
    int width_  = 0;
    int height_ = 0;

public:
    // デフォルトコンストラクタ
    Texture() = default;
    // デストラクタはリソース解放のため定義する
    ~Texture();
    // コピー禁止
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    // ムーブは許可
    Texture(Texture&& other) noexcept{
        Swap(other);   // 所有権を奪われたので無効化
    }
    Texture& operator=(Texture&& other) noexcept{
        // 異なるtexture_への=演算子によるムーブ
        if(this != &other){
            Reset();    // 左辺のtexture終了処理
            Swap(other);
        }
        return *this;   // ムーブされたメンバを返却
    }

    // 画像ファイル読み込み
    void Load(
        SDL_Renderer* renderer, 
        const std::filesystem::path& path
    );
    void Reset() noexcept;
    
    // getter
    SDL_Texture* Get() const noexcept{
        return texture_;
    }
    int Width() const noexcept{
        return width_;
    }
    int Height() const noexcept{
        return height_;
    }
    bool IsValid() const noexcept{
        return texture_ != nullptr;
    }
private:
    // ムーブ用に左辺と右辺の値を入れ替える関数
    void Swap(Texture& other) noexcept{
        std::swap(texture_, other.texture_);
        std::swap(width_,   other.width_);
        std::swap(height_,  other.height_);
    }
};
}   // namespace zimovka

#endif  // ZIMOVKA_RENDERING_TEXTURE_HPP_
