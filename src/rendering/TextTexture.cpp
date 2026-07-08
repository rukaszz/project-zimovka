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
    // キャッシュも移動させる
    , cached_renderer_(other.cached_renderer_)
    , cached_font_(other.cached_font_)
    , cached_text_(std::move(other.cached_text_))
    , cached_color_(other.cached_color_)
{
    // 移動元の所有権を無効化(デストラクタでSDL_DestroyTextureが呼ばれないようにするため)
    other.texture_         = nullptr;
    other.cached_renderer_ = nullptr;
    other.cached_font_     = nullptr;
    other.width_           = 0;
    other.height_          = 0;
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
        texture_          = other.texture_;
        width_            = other.width_;
        height_           = other.height_;
        cached_renderer_  = other.cached_renderer_;
        cached_font_      = other.cached_font_;
        cached_text_      = std::move(other.cached_text_);
        cached_color_     = other.cached_color_;
        // 移動元の所有権を無効化
        other.texture_          = nullptr;
        other.cached_renderer_  = nullptr;
        other.cached_font_      = nullptr;
        other.width_            = 0;
        other.height_           = 0;
    }
    return *this;
}

/**
 * @brief 文字列の更新処理(成功可否を返す)
 *
 * テキストと色が前回と同じ場合はSurface/Textureを再生成しない
 *
 * キャッシュヒットの条件:
 *   texture_ != nullptr (テクスチャが存在する)
 *   かつ text == cached_text_ (テキストが同じ)
 *   かつ color == cached_color_ (色が同じ)
 *
 * @param renderer 所有しない
 * @param font     所有しない
 * @param text     所有権を持たない文字列参照
 * @param color    内部でSDL_Colorに変換
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
    if (!renderer) {
        throw std::invalid_argument(
            "TextTexture requires a valid SDL_Renderer."
        );
    }
    if (!font) {
        SDL_Log("TextTexture::Update received a null font.");
        return false;
    }

    /**
     * @brief テクスチャが有効 かつ rendererが同一
     * かつ テキストが同一 かつ 色が同一 ならキャッシュを使用
     */
    if(texture_
        && renderer == cached_renderer_
        && font     == cached_font_
        && text     == cached_text_
        && color    == cached_color_)   // zimovka::Colorは==をオーバーロードしている
    {
        return true;
    }

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
    // 次回のキャッシュ判定用に保存
    texture_         = new_tex;
    cached_renderer_ = renderer;
    cached_font_     = font;
    cached_text_     = text;
    cached_color_    = color;  
    width_           = new_w;
    height_          = new_h;

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
void TextTexture::Render(int x, int y) const{
    // テクスチャ または レンダラが無効なら何もしない
    if(!texture_ || !cached_renderer_){
        return;
    }
    // 描画範囲
    const SDL_Rect dest{x, y, width_, height_};
    // レンダラへ送信
    SDL_RenderCopy(cached_renderer_, texture_, nullptr, &dest);
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
    cached_renderer_ = nullptr;
    cached_font_     = nullptr;
    cached_color_    = {};
    width_           = 0;
    height_          = 0;
    cached_text_.clear();
}

}   // namespace zimovka
