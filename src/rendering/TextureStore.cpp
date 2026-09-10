#include "zimovka/rendering/TextureStore.hpp"

#include <stdexcept>
#include <string_view>
#include <array>

namespace zimovka{

namespace{
// TextureId の順序に対応するファイル名
// TextureId::Count を変えたらここも合わせる
constexpr std::size_t TEXTURE_COUNT =
    static_cast<std::size_t>(TextureId::Count);

// TextureId → サブディレクトリ付きの相対パス
// ルートはLoadAll()で受け取る asset_root (assets_stab/ or assets/)
constexpr auto FILE_NAMES = std::to_array<std::string_view>({
    "img/player/player.png",                // TextureId::Player
    "img/enemy/enemy_prototype.png",        // TextureId::EnemyPrototype
    "img/bullet/player_bullet.png",         // TextureId::PlayerBullet
    "img/bullet/enemy_bullet.png",          // TextureId::EnemyBullet
    "img/background/stage1_background.png", // TextureId::Stage1Background
});
// ファイル追加忘れ防止用のassert
static_assert(
    FILE_NAMES.size() == static_cast<std::size_t>(TextureId::Count)
);
} // anonymous namespace

/**
 * @brief asset_root 以下の全テクスチャを読み込む
 *
 * @param renderer
 * @param asset_root  assets_stab/ など
 */
void TextureStore::LoadAll(
    SDL_Renderer* renderer,
    const std::filesystem::path& asset_root
){
    for(std::size_t i = 0; i < TEXTURE_COUNT; ++i){
        textures_[i].Load(renderer, asset_root / FILE_NAMES[i]);
    }
}

/**
 * @brief Texture IDに対応するテクスチャを返す
 *
 * @param id
 * @return const Texture&
 */
const Texture& TextureStore::GetTexture(TextureId id) const{
    const auto index = static_cast<std::size_t>(id);
    // 引数のidがtexturesより大きい or テクスチャの読み込みができていない
    if(index >= textures_.size() || !textures_[index].IsValid()){
        throw std::runtime_error("Requested texture is not loaded. ");
    }
    return textures_[index];
}

} // namespace zimovka
