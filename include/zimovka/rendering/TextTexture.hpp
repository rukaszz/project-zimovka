#ifndef ZIMOVKA_RENDERING_TEXTTEXTURE_HPP_
#define ZIMOVKA_RENDERING_TEXTTEXTURE_HPP_

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "zimovka/rendering/Color.hpp"

namespace zimovka{
/**
 * @brief 文字列のテクスチャを管理するクラス
 * 
 */
class TextTexture{
private:
    SDL_Texture* texture_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    // 毎フレーム更新しないためキャッシュする
    std::string cached_text_;
    Color       cached_color_{};

public:
    // コンストラクタ等の定義
    TextTexture() = default;
    ~TextTexture();
    // コピー禁止
    TextTexture(const TextTexture&) = delete;
    TextTexture& operator=(const TextTexture&) = delete;
    // ムーブ定義
    TextTexture(TextTexture&& other) noexcept;
    TextTexture& operator=(TextTexture&& other) noexcept;
    // 更新関数 (テキストが変化した場合のみSurface/Textureを再生成)
    bool Update(
        SDL_Renderer*       renderer,   // 所有しない
        TTF_Font*           font,       // 所有しない
        const std::string&  text,       // 所有権を持たない文字列参照
        zimovka::Color      color       // zimovka::Color (SDL_Colorへの変換は内部で実施)
    );
    // 描画関数
    void Render(SDL_Renderer* renderer, int x, int y) const;
    // リセット関数
    void Reset() noexcept;
};

}   // namespace zimovka

#endif  // ZIMOVKA_RENDERING_TEXTTEXTURE_HPP_
