#include "zimovka/rendering/overlay/GameTextOverlay.hpp"

#include <stdexcept>

#include "zimovka/config/ScreenLayout.hpp"
#include "zimovka/rendering/Color.hpp"
#include "zimovka/rendering/texture/TextTexture.hpp"

namespace zimovka{
/**
 * @brief Destroy the Game Text Overlay:: Game Text Overlay object
 * 
 * フォントはこのクラスで所有するので開放する
 */
GameTextOverlay::~GameTextOverlay(){
    if(font_){
        TTF_CloseFont(font_);
    }
}

/**
 * @brief フォント読み込みなどの初期化
 * 
 * @param renderer  : ポインタで参照し所有しない
 * @param font_path : フォントのパス
 * @param font_size : フォントサイズ
 */
void GameTextOverlay::Load(
    SDL_Renderer* renderer, 
    const std::string& font_path, 
    int font_size = 48
)
{
    // rendererのnullptrチェック
    if(!renderer){
        throw std::invalid_argument(
            "GameTextOverlay requires a valid SDL_Renderer. "
        );
    }
    renderer_ = renderer;

    // フォントオープン(rendererチェックでthrowが生じると開放されない可能性があるためここで確保)
    font_ = TTF_OpenFont(font_path.c_str(), font_size);
    // フォント読み込みエラーでもゲームは続ける
    if(!font_){
        SDL_Log("TTF_OpenFont failed: %s", TTF_GetError());
        return;
    }
}

/**
 * @brief 更新関数
 * 
 * NOTE: 現状はステージクリア描画関数
 * 
 * @param model 
 * @return true 
 * @return false 
 */
bool GameTextOverlay::Update(GameTextOverlayViewModel& model){
    // フォントの読み込み失敗時は何もしない
    if(!font_){
        return false;
    }
    // クリア状態でなければ即return
    // NOTE: 他の文字列を描画する際に条件は見直す
    if(!model.show_stage_clear){
        return false;
    }
    // 文字列構成
    constexpr zimovka::Color WHITE{255, 255, 255, 255};
    const std::string overlay_text = "STAGE CLEAR";
    // 描画成功で演出時間を更新
    if(overlay_text_.Update(renderer_, font_, overlay_text, WHITE)
    && model.stage_clear_ticks_remaining > 0){
        --model.stage_clear_ticks_remaining;
    }
    return true;
}

/**
 * @brief 描画関数
 * 
 */
void GameTextOverlay::Render() const{
    // フォントの読み込み失敗時は何もしない
    if(!font_){
        return;
    }
    // 描画位置
    const float X = ScreenLayout::PLAYFIELD_WIDTH  / 4;
    const float Y = ScreenLayout::PLAYFIELD_HEIGHT / 2;

    overlay_text_.Render(X, Y);
}
    
} // namespace zimovka
