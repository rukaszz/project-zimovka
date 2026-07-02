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
  , line_height_(font_size + 4) // +4は余白
{
    // font == nullptr → フォント読み込みが失敗している
    // ゲームは続行する
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
 * @brief 
 * 
 * @param text 
 * @param x 
 * @param y 
 */
void DebugOverlay::DrawLine(const std::string& text, int x, int y) const{
    // フォントが読み込めていなければ何もしない
    if(font_){
        return;
    }
}

}   // namespace zimovka
