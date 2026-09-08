#include "zimovka/engine/rendering/RenderPipeline.hpp"

#include "zimovka/rendering/TextureId.hpp"

namespace zimovka{
/**
 * @brief 規定の順序で各種の描画処理ｗｐ行う
 * 
 * @param gameplay: const取得用
 * @param sprites 
 * @param primitives 
 * @param texutures 
 * @param draw_debug_collision 
 */
void RenderPipeline::RenderTick(
    const UpdatePipeline& gameplay, 
    SpriteRenderer&       sprites, 
    PrimitiveRenderer&    primitives, 
    const TextureStore&   texutures, 
    bool draw_debug_collision
) const
{
    // player描画
    const auto& player = gameplay.GetPlayerSystem().GetPlayer();
    sprites.Draw(
        texutures.GetTexture(TextureId::Player), 
        {   // SpriteDrawParams
            .center = player.position, 
            .size   = {48.0f, 48.0f}, 
        }
    );

    // Enemy描画
    for(const Enemy& enemy : gameplay.GetEnemySystem().GetEnemies()){
        // 非active除外
        if(!enemy.active){
            continue;
        }
        sprites.Draw(
            texutures.GetTexture(TextureId::EnemyPrototype), 
            {   // SpriteDrawParams
                .center = enemy.position,
                .size   = enemy.render_size, 
            }
        );
        // デバッグ時は当たり判定円を描画
        if(draw_debug_collision){
            const Circle hurtbox = enemy.GetHurtboxCircle();
            primitives.DrawFilledCircle(
                hurtbox.center.x, 
                hurtbox.center.y, 
                hurtbox.radius, 
                {128, 32, 32, 255}
            );
        }
    }
    // Player Bullets
    for(const Bullet& pb : gameplay.GetPlayerBullets().GetBullets()){
        // 非activeは除外
        if(!pb.active){
            continue;
        }
        sprites.Draw(
            texutures.GetTexture(TextureId::PlayerBullet), 
            {   // SpriteDrawParams
                .center = pb.position, 
                .size   = {pb.radius, pb.radius}
            }
        );
    }
    // Enemy Bullets
    for(const Bullet& eb : gameplay.GetEnemyBullets().GetBullets()){
        // 非activeは除外
        if(!eb.active){
            continue;
        }
        sprites.Draw(
            texutures.GetTexture(TextureId::EnemyBullet), 
            {   // SpriteDrawParams
                .center = eb.position, 
                .size   = {eb.radius, eb.radius}
            }
        );
    }
}
} // namespace zimovka

