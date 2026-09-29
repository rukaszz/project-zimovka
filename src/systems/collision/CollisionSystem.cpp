#include "zimovka/systems/collision/CollisionSystem.hpp"

#include <cstdint>

#include "zimovka/systems/bullet/Bullet.hpp"
#include "zimovka/systems/enemy/Enemy.hpp"
#include "zimovka/systems/enemy/EnemyDamageResult.hpp"
#include "zimovka/systems/collision/CollisionUtilities.hpp"

namespace zimovka{
/**
 * @brief プレイヤー vs 弾の衝突判定を実施する関数
 * 
 * @param player 
 * @param bullets 
 * @return true 
 * @return false 
 */
bool CollisionSystem::CheckPlayerHitByBullets(
    const Player& player, 
    const BulletSystem& bullets
)
{
    // プレイヤーの中心座標を取得
    const Circle player_circle{
        player.position, 
        player.hit_radius
    };
    // bullets走査
    for(const Bullet& bullet : bullets.GetBullets()){
        // 非活性はスキップ
        if(!bullet.active){
            continue;
        }
        // 弾の中心座標
        const Circle bullet_circle{
            bullet.position, 
            bullet.radius
        };
        // 衝突判定回数計測
        ++collision_stats_.player_vs_enemy_bullet_checks;
        // 円同士の衝突判定
        if(CollisionUtilities::Intersects(player_circle, bullet_circle)){
            return true;
        }
    }
    // 走査して衝突していなければfalse
    return false;
}

/**
 * @brief 自機弾vs敵の衝突判定
 * ただし，両システムのvectorを直接書き換えない
 * 
 * @param player_bullets 
 * @param enemies 
 * @return true 
 * @return false 
 */
EnemyHitEvents CollisionSystem::ResolvePlayerBulletsVsEnemies(
    BulletSystem& player_bullets, EnemySystem& enemies
)
{
    // 結果(戻り値)
    EnemyHitEvents result{};
    // 最初にplayer_bulletsのループ
    const auto& bullets    = player_bullets.GetBullets();
    const auto& enemy_list = enemies.GetEnemies();

    for(std::size_t bullet_index = 0; bullet_index < bullets.size(); ++bullet_index){
        const Bullet& b = bullets[bullet_index];
        // 非活性は飛ばす
        if(!b.active){
            continue;
        }
        // 自機弾当たり判定(円)
        const Circle bullet_circle{
            b.position,
            b.radius
        };
        // enemiesのループ
        for(std::size_t enemy_index = 0; enemy_index < enemy_list.size(); ++enemy_index){
            const Enemy& e = enemy_list[enemy_index];
            // 非活性は飛ばす
            if(!e.active){
                continue;
            }
            // 判定回数計測
            ++collision_stats_.player_bullet_vs_enemy_checks;
            // 衝突判定
            if(!CollisionUtilities::Intersects(
                bullet_circle,
                e.GetHurtboxCircle()
            ))
            {
                // ヒットしていないなら次へ
                continue;
            }
            // 弾を非活性化
            player_bullets.Deactivate(bullet_index);
            // ダメージ処理: 結果に応じてhit/killをカウント
            const EnemyDamageResult damage_result = enemies.TakeDamage(enemy_index, 1);
            // ダメージが通らないなら次へ
            if (damage_result == EnemyDamageResult::InvalidTarget) {
                continue;
            }

            ++result.hit_count;
            
            if (damage_result == EnemyDamageResult::Destroyed) {
                ++result.kill_count;
            }
            // 弾が当たったので次の弾へ(貫通しない)
            break;
        }
    }
    // 全走査して結果を返す
    return result;
}

/**
 * @brief 自機弾vs敵弾の衝突解決
 * 自機弾によって敵弾を打ち消せる
 * 
 * @param player_bullets 
 * @param enemy_bullets 
 * @return BulletCancelEvents 
 */
BulletCancelEvents CollisionSystem::ResolvePlayerBulletsVsEnemyBullets(
    BulletSystem& player_bullets, 
    BulletSystem& enemy_bullets
)
{
    // 返却値
    BulletCancelEvents result{};
    // 両Bulletsを取得
    const auto& player = player_bullets.GetBullets();
    const auto& enemy  = enemy_bullets.GetBullets();
    // 最初に自機弾でループ
    for(std::size_t pb_index = 0; pb_index < player.size(); ++pb_index){
        const Bullet& pb = player[pb_index];
        // 非活性は飛ばす
        if(!pb.active){
            continue;
        }
        // 自機弾当たり判定(円)
        const Circle pb_circle{
            pb.position,
            pb.radius
        };
        // 敵弾のループ
        for(std::size_t eb_index = 0; eb_index < enemy.size(); ++eb_index){
            const Bullet& eb = enemy[eb_index];
            // 非活性は飛ばす
            if(!eb.active){
                continue;
            }
            // 敵弾当たり判定(円)
            const Circle eb_circle{
                eb.position,
                eb.radius
            };
            // 判定回数計測
            ++collision_stats_.player_bullet_vs_enemy_checks;
            // 衝突判定
            if(!CollisionUtilities::Intersects(
                pb_circle,
                eb_circle
            ))
            {
                // ヒットしていないなら次へ
                continue;
            }
            // 両弾を非活性化
            player_bullets.Deactivate(pb_index);
            enemy_bullets.Deactivate(eb_index);

            ++result.cancel_count;
            
            // 弾があたったら次へ
            break;
        }
    }
    // 全走査して結果を返す
    return result;
}


}   // namespace zimovka
