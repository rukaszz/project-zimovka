#ifndef ZIMOVKA_DEBUG_DEBUGOVERLAY_HPP_
#define ZIMOVKA_DEBUG_DEBUGOVERLAY_HPP_

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "zimovka/debug/DebugStats.hpp"

namespace zimovka{

/**
 * @brief デバッグ情報をSDL_ttfを用いて描画するクラス
 * 
 */
class DebugOverlay{
private:
    SDL_Renderer* renderer_;    // 所有しない
    TTF_Font*     font_;        // 所有する
    int           line_height_; // フォントサイズや余白の定義

    // 1行のテキストを描画するユーティリティ関数
    void DrawLine(const std::string& text, int x, int y) const;

public:
    // コンストラクタ(フォントサイズはデフォルト14にする)
    DebugOverlay(
        SDL_Renderer* renderer, 
        const std::string& font_path, 
        int font_size = 14
    );
    ~DebugOverlay();

    // TTF_Fontを所有するのでコピー禁止
    DebugOverlay(const DebugOverlay&) = delete;
    DebugOverlay& operator=(const DebugOverlay&) = delete;

    // 描画関数
    void Render(const DebugStats& stats) const;
    // TTF_Fontが有効なのか
    bool IsLoaded() const noexcept{
        return font_ != nullptr;
    } 
};

}   // namespace zimovka

#endif  // ZIMOVKA_DEBUG_DEBUGOVERLAY_HPP_
