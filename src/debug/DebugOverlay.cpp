#include "zimovka/debug/DebugOverlay.hpp"

#include <format>

namespace zimovka{

/**
 * @brief Construct a new Debug Overlay:: Debug Overlay object
 * 
 * @param renderer: これはポインタで参照し所有はしない
 * @param font_path: フォントの配置場所 
 * @param font_size: フォントのサイズ(余白は考慮していない) 
 */
DebugOverlay::DebugOverlay(
    SDL_Renderer* renderer, 
    const std::string& font_path, 
    int font_size
) : renderer_(renderer)
  , font_(TTF_OpenFont(font_path.c_str(), font_size))
{
    // rendererのnullptrチェック
    if(!renderer_){
        throw std::invalid_argument(
            "DebugOverlay requires a valid SDL_Renderer."
        );
    }
    // font == nullptr → フォント読み込みが失敗している
    // ゲームは続行する
    if(!font_){
        SDL_Log("TTF_OpenFont failed: %s", TTF_GetError());
    } else {
        // font_が読み込み成功していれば，line_height_を設定(+2は余白)
        // ※TTF_FontLineSkip(NULL)だとセグメンテーション違反となるため
        line_height_ = TTF_FontLineSkip(font_) + 2;
    }
} 

/**
 * @brief Destroy the Debug Overlay:: Debug Overlay object
 * 
 * 呼ばれた際はフォントを開放する
 */
DebugOverlay::~DebugOverlay(){
    if(font_){
        TTF_CloseFont(font_);
    }
}

/**
 * @brief 描画用にテクスチャを生成してレンダラへ送る処理
 * 
 * @param text 
 * @param x 
 * @param y 
 */
void DebugOverlay::DrawLine(const std::string& text, int x, int y) const{
    // フォントが読み込めていなければ何もしない
    if(!font_){
        return;
    }
    // SDL_ttfでtext→surface→textureに変換して描画
    constexpr SDL_Color YELLOW{255, 255, 0, 255};

    // SDL_Surfaceの生成
    SDL_Surface* surf = TTF_RenderUTF8_Solid(font_, text.c_str(), YELLOW);
    if(!surf){
        return;
    }

    // SDL_Texture生成
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surf);
    SDL_FreeSurface(surf);  // textureの生成処理が完了したら即surfaceを解放
    if(!tex){
        return;
    }

    // textの描画サイズをテクスチャから取得
    int w = 0, h = 0;
    /**
     * @brief テクスチャの情報取得(調査対象に対して，引数で与えられる情報を取れる)
     * arg1: texture(調査対象)
     * arg2: format(ピクセルの形式に関する構造体ポインタ)NULL可
     * arg3: access(アクセスするポインタ)NULL可
     * arg4, 5: テクスチャの幅/高さ．NULL可
     */
    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
    const SDL_Rect dest{x, y, w, h};    // 描画範囲
    SDL_RenderCopy(renderer_, tex, nullptr, &dest); // 描画
    SDL_DestroyTexture(tex);    // 描画が完了したら即開放
}

/**
 * @brief デバッグ情報の描画
 * 
 * @param stats 
 */
void DebugOverlay::Render(const DebugStats& stats) const{
    // フォントの読み込み失敗時はなにもしない
    if(!font_){
        return;
    }
    // 描画用の座標
    constexpr int X = 8;
    int y = 8;

    // 各種の処理時間(平均/最大)を描画
    // avg/maxは0.25s区間の平均値・最大値
    DrawLine(std::format("frame:    avg {:6.2f}  max {:6.2f} ms", stats.frame_ms,      stats.max_frame_ms),      X, y);
    y += line_height_;
    DrawLine(std::format("process:  avg {:6.2f}  max {:6.2f} ms", stats.processing_ms, stats.max_processing_ms), X, y);
    y += line_height_;
    DrawLine(std::format("update:   avg {:6.2f}  max {:6.2f} ms", stats.update_ms,     stats.max_update_ms),     X, y);
    y += line_height_;
    DrawLine(std::format("render:   avg {:6.2f}  max {:6.2f} ms", stats.render_ms,     stats.max_render_ms),     X, y);
    y += line_height_;
    DrawLine(std::format("steps: {}", stats.update_steps), X, y);
    y += line_height_;
    // 弾，衝突処理のチェック
    DrawLine(std::format("bullets: {} / {}", stats.active_bullets, stats.bullet_capacity), X, y);
    y += line_height_;
    DrawLine(std::format("collision: {} checks", stats.collision_checks), X, y);
}

}   // namespace zimovka
