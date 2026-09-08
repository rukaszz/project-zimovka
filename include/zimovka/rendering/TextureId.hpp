#ifndef ZIMOVKA_RENDERING_TEXTUREID_HPP_
#define ZIMOVKA_RENDERING_TEXTUREID_HPP_

namespace zimovka{
/**
 * @brief 管理するテクスチャのID管理用enum
 * 
 */
enum class TextureId{
    Player, 
    EnemyPrototype, 
    PlayerBullet, 
    EnemyBullet,
    Stage1Background, 
    
    Count, // 門番
};
}   // namespace zimovka

#endif  // ZIMOVKA_RENDERING_TEXTUREID_HPP_
