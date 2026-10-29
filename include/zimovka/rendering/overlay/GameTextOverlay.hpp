#ifndef ZIMOVKA_RENDERING_OVERLAY_GAMETEXTOVERLAY_HPP_
#define ZIMOVKA_RENDERING_OVERLAY_GAMETEXTOVERLAY_HPP_

#include <string>

#include <SDL2/SDL_ttf.h>

#include "zimovka/rendering/overlay/GameTextOverlayViewModel.hpp"
#include "zimovka/rendering/texture/TextTexture.hpp"

namespace zimovka{
/**
 * @brief 主にプレイフィールドに文字を描画するためのクラス
 * 
 */
class GameTextOverlay{
private:
    // fontはこのクラスが所有するので開放する義務を負う
    SDL_Renderer* renderer_ = nullptr;  // 所有しない
    TTF_Font*     font_     = nullptr;  // 所有する

    TextTexture overlay_text_;
public:
    GameTextOverlay(
        SDL_Renderer* renderer, 
        const std::string& font_path, 
        int font_size = 48
    );
    ~GameTextOverlay();
    // TTF_Font所有の関係でコピー禁止
    GameTextOverlay(const GameTextOverlay&) = delete;
    GameTextOverlay& operator=(const GameTextOverlay&) = delete;

    // 更新関数(TextTexture.Update()の戻り値に従う)
    bool Update(GameTextOverlayViewModel& model);
    // 描画関数
    void Render() const;
    // TTF_Fontが有効化
    bool IsLoaded() const noexcept{
        return font_ != nullptr;
    }

};
} // namespace zimovka

#endif  // ZIMOVKA_RENDERING_OVERLAY_GAMETEXTOVERLAY_HPP_
