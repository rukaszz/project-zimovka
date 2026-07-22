#ifndef ZIMOVKA_DEBUG_DEBUGOVERLAY_HPP_
#define ZIMOVKA_DEBUG_DEBUGOVERLAY_HPP_

#include <array>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "zimovka/debug/DebugStats.hpp"
#include "zimovka/rendering/TextTexture.hpp"

namespace zimovka{

/**
 * @brief デバッグ情報をSDL_ttfを用いて描画するクラス
 * 
 */
class DebugOverlay{
public:
    // デバッグ情報の描画行数
    static constexpr std::size_t LINE_COUNT = 13;
private:
    SDL_Renderer* renderer_    = nullptr;   // 所有しない
    TTF_Font*     font_        = nullptr;   // 所有する
    int           line_height_ = 0;         // フォントサイズや余白の定義
    // デバッグ情報の描画時に使用するテキスト用テクスチャ管理行列
    std::array<TextTexture, LINE_COUNT> lines_{};

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
    // 更新関数(TextTexture.Update()がboolを返すので，このUpdateもboolを返す)
    bool Update(const DebugStats& stats);
    // 描画関数
    void Render() const;
    // TTF_Fontが有効なのか
    bool IsLoaded() const noexcept{
        return font_ != nullptr;
    } 
};

}   // namespace zimovka

#endif  // ZIMOVKA_DEBUG_DEBUGOVERLAY_HPP_
