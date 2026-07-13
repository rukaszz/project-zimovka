#include "zimovka/debug/DebugOverlay.hpp"

#include <format>
#include <stdexcept>

#include "zimovka/rendering/Color.hpp"

namespace zimovka{

/**
 * @brief Construct a new Debug Overlay:: Debug Overlay object
 * 
 * @param renderer:  これはポインタで参照し所有はしない
 * @param font_path: フォントの配置場所 
 * @param font_size: フォントのサイズ(余白は考慮していない) 
 */
DebugOverlay::DebugOverlay(
    SDL_Renderer* renderer, 
    const std::string& font_path, 
    int font_size
) : renderer_(renderer)
  , font_(nullptr)
  , line_height_(0)
{
    // rendererのnullptrチェック
    if(!renderer_){
        throw std::invalid_argument(
            "DebugOverlay requires a valid SDL_Renderer."
        );
    }
    // フォントオープン(rendererがnullptrだと例外throw → 解放されない可能性があるのでここでオープン)
    font_ = TTF_OpenFont(font_path.c_str(), font_size);
    // font == nullptr → フォント読み込みが失敗している
    // ゲームは続行する
    if(!font_){
        SDL_Log("TTF_OpenFont failed: %s", TTF_GetError());
        return;
    }
    // font_が読み込み成功していれば，line_height_を設定(+2は余白)
    // ※TTF_FontLineSkip(NULL)だとセグメンテーション違反となるため
    line_height_ = TTF_FontLineSkip(font_) + 2;
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
 * @brief 更新関数
 * 
 * デバッグ情報描画用の文字列をキャッシュ行列へ格納する
 * 
 * @param stats 
 */
bool DebugOverlay::Update(const DebugStats& stats){
    // フォントが読み込めていないなら何もしない
    if(!font_){
        return false;
    }
    constexpr zimovka::Color YELLOW{255, 255, 0, 255};

    // lines_へ格納する文字列textの作成(LINE_COUNT行分)
    const std::array<std::string, LINE_COUNT> texts{
        std::format(
            "frame:     avg{:6.2f}, max{:6.2f} ms", 
            stats.frame_ms, 
            stats.max_frame_ms
        ), 
        std::format(
            "process:   avg{:6.2f}, max{:6.2f} ms", 
            stats.processing_ms, 
            stats.max_processing_ms
        ), 
        std::format(
            "update:    avg{:6.2f}, max{:6.2f} ms", 
            stats.update_ms, 
            stats.max_update_ms
        ), 
        std::format(
            "render:    avg{:6.2f}, max{:6.2f} ms", 
            stats.render_ms, 
            stats.max_render_ms
        ), 
        std::format(
            "steps:     max {}, zero {}, multi {}", 
            stats.max_update_steps, 
            stats.zero_update_frames, 
            stats.multi_update_frames
        ), 
        std::format(
            "bullets:   {} / {}", 
            stats.active_bullets, 
            stats.bullet_capacity
        ), 
        std::format(
            "collision: {} checks", 
            stats.collision_checks
        ), 
    };
    
    // Update処理の成否
    bool success = true;
    // lines_へ格納
    for(std::size_t i = 0; i < lines_.size(); ++i){
        // TextTextureのUpdate()を呼び出して文字列のテクスチャを設定
        // success = Update() && successにすることで，どこかで失敗したらfalseを取れる
        success = lines_[i].Update(renderer_, font_, texts[i], YELLOW) && success;
    }
    return success;
}

/**
 * @brief デバッグ情報の描画
 * 
 * @param stats 
 */
void DebugOverlay::Render() const{
    // フォントの読み込み失敗時はなにもしない
    if(!font_){
        return;
    }
    // 描画用の座標
    constexpr int X   = 8;
    constexpr int TOP = 8;  // ここから縦にずれていく
    
    // TextTextureのRender()で描画
    for(std::size_t i = 0; i < lines_.size(); ++i){
        const int y = TOP + static_cast<int>(i) * line_height_;
        (void)lines_[i].Render(X, y);   // Debug用なのでTextTextureの戻り値は無視する
    }
}

}   // namespace zimovka
