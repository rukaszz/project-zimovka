#include "zimovka/rendering/Texture.hpp"

#include <stdexcept>
#include <string>

#include <SDL2/SDL_image.h>

namespace zimovka{
/**
 * @brief Destroy the Texture:: Texture object
 * 
 */
Texture::~Texture(){
    Reset();
}

/**
 * @brief 画像からtextureを作成する
 * 
 * @param renderer 
 * @param path 
 */
void Texture::Load(
    SDL_Renderer* renderer, 
    const std::filesystem::path& path
)
{
    // 引数チェック
    if(!renderer){
        throw std::invalid_argument(
            "Texture::Load renderer is null. "
        );
    }
    // Linux/Windows双方で対応できるようにpath_stringで受ける
    const std::string path_string = path.string();
    // Surface: RAMに保存されたピクセルデータ管理用構造体
    SDL_Surface* surf = IMG_Load(path_string.c_str());
    // 読み込みできなければ終了
    if(!surf){
        throw std::runtime_error(std::string("IMG_Load failed: ") + path.string() + " | " + IMG_GetError());
    }
    // surface→texureを構成
    // まず変数で受け取る(再Loadでのリーク防止用)
    SDL_Texture* new_texture = SDL_CreateTextureFromSurface(renderer, surf);
    // texture_ = SDL_CreateTextureFromSurface(renderer, surf);
    if(!new_texture){
        SDL_FreeSurface(surf);  // surfaceを解放しておく
        throw std::runtime_error(std::string("Texture creation failed. ") + SDL_GetError());
    }
    // メンバ変数設定(リーク防止用の仮)
    const int new_width  = surf->w;
    const int new_height = surf->h;
    SDL_FreeSurface(surf);

    // ここまで処理が成功してから旧Textureを破棄
    Reset();
    // 新Textureをセット
    texture_ = new_texture;
    width_   = new_width;
    height_  = new_height;
}

/**
 * @brief textureの破棄処理
 * 
 */
void Texture::Reset() noexcept{
    if(texture_){
        SDL_DestroyTexture(texture_);
        texture_ = nullptr; // メンバ変数も無効化
    }
    width_  = 0;
    height_ = 0;
}

}   // namespace zimovka
