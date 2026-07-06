#include "zimovka/rendering/TextTexture.hpp"

#include <stdexcept>
#include <utility>

namespace zimovka{

/**
 * @brief Destroy the Text Texture:: Text Texture object
 * 
 * Reset()がリソース解放を担っている
 */
TextTexture::~TextTexture(){
    Reset();
}

/**
 * @brief ムーブコンストラクタ
 * 
 * @param other 
 */
TextTexture::TextTexture(TextTexture&& other) noexcept
    : texture_(other.texture_)
    , width_(other.width_)
    , height_(other.height_)
    , cached_text_(std::move(other.cached_text_))
{
    // 移動元の所有権を無効化(デストラクタでSDL_DestroyTextureが呼ばれないようにするため)
    other.texture_ = nullptr;
    other.width_   = 0;
    other.height_  = 0;
}

/**
 * @brief ムーブ代入演算子
 * 
 * @param other 
 * @return TextTexture& 
 */
TextTexture& TextTexture::operator=(TextTexture&& other) noexcept{
    if(this != &other){
        // 既存テクスチャを解放してから移動
        Reset();
        texture_       = other.texture_;
        width_         = other.width_;
        height_        = other.height_;
        cached_text_   = std::move(other.cached_text_);  // string_viewではないのでOK
        // 移動元の所有権を無効化
        other.texture_ = nullptr;
        other.width_   = 0;
        other.height_  = 0;
    }
    return *this;
}

/**
 * @brief 文字列の更新処理(成功可否を返す)
 * 
 * テキストが変化した場合のみSurface→Textureを再生成する
 * 
 * @param renderer 所有しない
 * @param font     所有しない
 * @param text     string_viewで所有権を持たない
 * @param color    内部実装のcolorを受け取り内部でSDL_Colorに変換
 * @return true 
 * @return false 
 */
bool TextTexture::Update(
    SDL_Renderer*       renderer,
    TTF_Font*           font,
    const std::string&  text,
    zimovka::Color      color
)
{
    // チェック
    if (!renderer) {
        throw std::invalid_argument(
            "TextTexture requires a valid SDL_Renderer."
        );
    }
    // フォントエラーの際はゲーム事態は継続
    if (!font) {
        SDL_Log("TextTexture::Update received a null font.");
        return false;
    }
    // 同色か判定
    const bool same_color = has_cached_color_
        && color.r == cached_color_.r
        && color.g == cached_color_.g
        && color.b == cached_color_.b
        && color.a == cached_color_.a;
    // テクスチャが有効 かつ 同一の文字列 かつ 同色 なら再生成しない
    if(texture_ && text == cached_text_ && same_color){
        return true;
    }

    // キャッシュの判定を超えたのでfalse
    has_cached_color_ = false;

    // zimovka::Color → SDL_Color 変換
    const SDL_Color sdl_color{color.r, color.g, color.b, color.a};

    // Surface生成
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), sdl_color);
    if(!surf){
        return false;
    }

    // Texture生成 (サイズをSurface破棄前に取得)
    SDL_Texture* new_tex = SDL_CreateTextureFromSurface(renderer, surf);
    const int new_w = surf->w;
    const int new_h = surf->h;
    SDL_FreeSurface(surf);  // Texture生成処理完了後に即破棄
    if(!new_tex){
        return false;
    }

    // 既存テクスチャを解放してから更新
    Reset();
    texture_     = new_tex;
    width_       = new_w;
    height_      = new_h;
    cached_text_ = text;

    return true;
}

/**
 * @brief 描画処理(RendererCopyのラッパ)
 * 
 * テクスチャが未設定の場合は何もしない
 * 
 * @param renderer 
 * @param x 
 * @param y 
 */
void TextTexture::Render(SDL_Renderer* renderer, int x, int y) const{
    // テクスチャが無効なら何もしない
    if(!texture_){
        return;
    }
    if (!renderer) {
        throw std::invalid_argument(
            "DebugOverlay requires a valid SDL_Renderer."
        );
    }
    // 描画範囲
    const SDL_Rect dest{x, y, width_, height_};
    // レンダラへ送信
    SDL_RenderCopy(renderer, texture_, nullptr, &dest);
}

/**
 * @brief リセット関数(テクスチャを解放して初期状態に戻す)
 * 
 */
void TextTexture::Reset() noexcept{
    if(texture_){
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    width_       = 0;
    height_      = 0;
    cached_text_.clear();
}

}   // namespace zimovka
