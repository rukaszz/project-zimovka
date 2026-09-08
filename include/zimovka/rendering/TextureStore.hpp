#ifndef ZIMOVKA_RENDERING_TEXTURESTORE_HPP_
#define ZIMOVKA_RENDERING_TEXTURESTORE_HPP_

#include <array>
#include <cstddef>
#include <filesystem>

#include <SDL2/SDL.h>

#include "zimovka/rendering/Texture.hpp"
#include "zimovka/rendering/TextureId.hpp"

namespace zimovka{
/**
 * @brief 各テクスチャを管理するクラス
 * 
 * 全Textureはゲーム開始前にすべてする読み込む
 */
class TextureStore{
private:
    static constexpr std::size_t COUNT = 
        static_cast<std::size_t>(TextureId::Count);
    // 現状は固定なのでHash Map(unordered_map)ではなくarray
    std::array<Texture, COUNT> textures_{};

public:
    // 全assetsの読み込み
    void LoadAll(
        SDL_Renderer* renderer, 
        const std::filesystem::path& asset_root
    );
    const Texture& GetTexture(TextureId id) const;
};
}

#endif  // ZIMOVKA_RENDERING_TEXTURESTORE_HPP_
