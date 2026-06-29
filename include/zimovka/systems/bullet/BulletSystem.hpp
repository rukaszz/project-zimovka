#ifndef ZIMOVKA_SYSTEMS_BULLET_BULLETSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_BULLET_BULLETSYSTEM_HPP_

#include <array>
#include <cstddef>
#include <vector>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/bullet/Bullet.hpp"

namespace zimovka{

class PrimitiveRenderer;

/**
 * @brief AoSに基づいて弾を連続メモリで管理するクラス
 * 
 */
class BulletSystem{
private:
    // 定数
    static constexpr std::size_t MAX_ENEMY_BULLETS = 1200;
    static constexpr std::size_t MAX_PLAYER_BULLETS = 120;
    // AoSに基づいた配列による弾管理
    std::vector<Bullet> bullets_;
    // std::array<Bullet, MAX_ENEMY_BULLETS> bullets_;
    std::size_t next_spawn_idx_ = 0;

    // 画面外判定
    bool IsOutOfScreen(const Bullet& bullet, float screen_width, float screen_height);

public:
    explicit BulletSystem(std::size_t max_bullets = 1200);

    // 弾出現
    bool Spawn(const Vec2& position, const Vec2& velociry, float radius);
    // 更新処理
    void Update(float dt, float screen_width, float screen_height);
    // 描画
    void Render(PrimitiveRenderer& renderer) const;
    
    // 活性状態の弾数
    std::size_t CountActive() const noexcept;
    // 描画可能最大弾数
    std::size_t Capaciry() const noexcept{
        return bullets_.size();
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BULLET_BULLETSYSTEM_HPP_
