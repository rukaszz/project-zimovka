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
    Reset();
}

/**
 * @brief リセット関数
 * 
 * GameTextOverlayが生きている際に再読み込み等を実装する際にメンバを初期化する
 */
void GameTextOverlay::Reset() noexcept{
    // TextTextureメンバ変数をリセット※Rendererが生存していないといけない
    stage_clear_text_.Reset();
    
    // SDL関係のリソース解放
    if(font_){
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if(renderer_){
        renderer_ = nullptr;
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
    int font_size
)
{
    // 引数チェック
    if(!renderer){
        throw std::invalid_argument(
            "GameTextOverlay requires a valid SDL_Renderer. "
        );
    }
    if(font_size <= 0){
        throw std::invalid_argument(
            "GameTextOverlay requires no less than 1. "
        );
    }

    // フォントの入れ替えを考慮してfont_を設定
    // フォント読み込み
    TTF_Font* new_font = TTF_OpenFont(font_path.c_str(), font_size);
    if(!new_font){
        throw std::runtime_error(
            std::string("TTF_OpenFont failed: ") + TTF_GetError()
        );
    }

    // 既存リソースを破棄して更新
    Reset();
    
    renderer_ = renderer;
    font_     = new_font;
}

/**
 * @brief 描画関数
 * 
 * この関数内で表示判定やTextTexture.Update()を行う
 */
bool GameTextOverlay::Render(const GameTextOverlayViewModel& model){
    // クリア状態でなければ即return
    // NOTE: 他の文字列を描画する際に条件は見直す
    if(!model.show_stage_clear){
        return true;
    }
    if(!renderer_ || !font_){
        return false;
    }
    constexpr zimovka::Color WHITE{
        255, 255, 255, 255
    };
    // TextTexture::Update()の実施：テクスチャ生成などが失敗したらfalseを伝搬
    if(!stage_clear_text_.Update(
        renderer_, 
        font_, 
        "STAGE CLEAR", 
        WHITE
    ))
    {
        return false;
    }
    // 描画位置
    const int x = static_cast<int>(
        ScreenLayout::PLAYFIELD_X + (ScreenLayout::PLAYFIELD_WIDTH - stage_clear_text_.Width()) / 2.0f
    );
    const int y = static_cast<int>(
        ScreenLayout::PLAYFIELD_Y + (ScreenLayout::PLAYFIELD_HEIGHT - stage_clear_text_.Height()) / 2.0f
    );
    return stage_clear_text_.Render(x, y);
}
    
} // namespace zimovka
