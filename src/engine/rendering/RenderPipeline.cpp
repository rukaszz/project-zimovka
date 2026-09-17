#include "zimovka/engine/rendering/RenderPipeline.hpp"

#include "zimovka/rendering/hud/HudRenderer.hpp"
#include "zimovka/rendering/hud/HudViewModel.hpp"
#include "zimovka/rendering/texture/TextureId.hpp"

namespace zimovka{
// 仮定数
namespace{
    constexpr Vec2 PLAYER_BULLET_SIZE{
        8.0f, 16.0f
    };
    constexpr Vec2 ENEMY_BULLET_SIZE{
        14.0f, 14.0f
    };
}   // anonymous namespace

/**
 * @brief 規定の順序で各種の描画処理を行う
 * 
 * 処理順序は
 * Background
 * Enemy
 * Player Bullet
 * Enemy Bullet
 * Player
 * Effects
 * Debug collision
 * HUD
 * 
 * @param gameplay: const取得用
 * @param sprites 
 * @param primitives 
 * @param texutures 
 * @param draw_debug_collision 
 */
void RenderPipeline::RenderFrame(
    const UpdatePipeline& gameplay, 
    SpriteRenderer&       sprites, 
    PrimitiveRenderer&    primitives, 
    const TextureStore&   texutures, 
    bool draw_debug_collision
) const
{
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
                .size   = PLAYER_BULLET_SIZE
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
                .size   = ENEMY_BULLET_SIZE
            }
        );
    }
    // Player描画
    const auto& player = gameplay.GetPlayerSystem().GetPlayer();
    sprites.Draw(
        texutures.GetTexture(TextureId::Player),
        {   // SpriteDrawParams
            .center = player.position,
            .size   = {48.0f, 48.0f},
        }
    );

    // ── HUD描画 ──────────────────────────────────────────
    // UpdatePipeline から表示用データのスナップショットを構築する
    const auto& weapon_state  = gameplay.GetPlayerWeaponSystem().GetState();
    const auto& weapon_config = gameplay.GetPlayerWeaponSystem().GetConfig();
    const auto& bomb_state    = gameplay.GetBombSystem().GetState();

    HudViewModel hud_model;
    hud_model.ammo                    = weapon_state.ammo;
    hud_model.max_ammo                = weapon_config.max_ammo;
    hud_model.reloading               = weapon_state.IsReloading();
    hud_model.reload_ticks_remaining  = weapon_state.reload_ticks_remaining;
    hud_model.reload_duration_ticks   = weapon_config.reload_duration_ticks;
    hud_model.bomb_stock              = bomb_state.stock;

    HudRenderer hud_renderer;
    hud_renderer.Render(hud_model, primitives);
}
} // namespace zimovka

