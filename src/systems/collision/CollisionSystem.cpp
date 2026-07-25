#include "zimovka/systems/collision/CollisionSystem.hpp"

#include <cstdint>

#include "zimovka/systems/bullet/Bullet.hpp"
#include "zimovka/systems/enemy/Enemy.hpp"
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
    // 初期化
    last_check_count_ = 0;
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
        // 活性のbulletsを調べるので+1
        ++last_check_count_;
        // 弾の中心座標
        const Circle bullet_circle{
            bullet.position, 
            bullet.radius
        };
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
bool CollisionSystem::ResolvePlayerBulletVsEnemies(
    BulletSystem& player_bullets, EnemySystem& enemies
)
{
    // 弾が当たったかどうかの判定(戻り値)
    bool any_hit = false;
    // 初期化
    last_check_count_ = 0;
    // 最初にplayer_bulletsのループ
    const auto& bullets    = player_bullets.GetBullets();
    const auto& enemy_list = enemies.GetEnemies();

    for(std::size_t bullet_index = 0; bullet_index < bullets.size(); ++bullet_index){
        // player_bulletの配列の要素へアクセス
        const Bullet& b = bullets[bullet_index];
        // 非活性は飛ばす
        if(!b.active){
            continue;
        }
        // 活性のbulletsを調べるので+1
        ++last_check_count_;
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
            // 敵の当たり判定(円)
            const Circle enemy_hurtbox{
                e.position + e.hurtbox_offset, 
                e.hurtbox_radius
            };
            // 衝突判定
            if(!CollisionUtilities::Intersects(
                bullet_circle, 
                enemy_hurtbox
            ))
            {
                // ヒットしていないなら次へ
                continue;   
            }
            // それぞれのシステムへヒットしたインデックスを伝搬
            player_bullets.Deactivate(bullet_index);
            enemies.TakeDamage(enemy_index, 1);
            any_hit = true;
            // 弾が当たったので次の弾へ(貫通しない)
            break;
        }
    }
    // 全走査して結果を返す
    return any_hit;
}


}   // namespace zimovka
