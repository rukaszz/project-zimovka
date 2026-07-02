#include "zimovka/systems/collision/CollisionSystem.hpp"

#include "zimovka/systems/bullet/Bullet.hpp"
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
        if(zimovka::CollisionUtilities::Intersects(player_circle, bullet_circle)){
            return true;
        }
    }
    // 走査して衝突していなければfalse
    return false;
}

}   // namespace zimovka
