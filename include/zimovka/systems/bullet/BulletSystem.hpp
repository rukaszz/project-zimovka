#ifndef ZIMOVKA_SYSTEMS_BULLET_BULLETSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_BULLET_BULLETSYSTEM_HPP_

#include <cstddef>
#include <vector>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/rendering/Color.hpp"
#include "zimovka/systems/bullet/Bullet.hpp"

namespace zimovka{

class PrimitiveRenderer;

/**
 * @brief AoSに基づいて弾を連続メモリで管理するシステム
 *
 * 弾の生成は inactive→active の状態遷移であり，オブジェクト生成(ヒープ)は一切使わない
 * ゲーム開始時に最大弾数分のメモリを確保して，ゲームループ中のヒープ確保はゼロにする
 *
 */
class BulletSystem{
public:
    static constexpr std::size_t DEFAULT_MAX_BULLETS = 1200;

    explicit BulletSystem(std::size_t max_bullets = DEFAULT_MAX_BULLETS);

    /**
     * @brief 弾を生成する（inactive→active の状態遷移）
     *
     * next_spawn_idx_から循環して空きスロットを探す
     * プールが満杯なら生成できないのでfalseとなる
     *
     * @param position  初期座標
     * @param velocity  速度 (px/s)
     * @param radius    弾半径
     * @param color     弾色
     * @return true 生成成功 / false プール満杯
     */
    bool Spawn(const Vec2& position, const Vec2& velocity, float radius,
               Color color = Color{255, 100, 100, 255});

    // 更新処理（移動・画面外消去）
    void Update(float dt, float screen_width, float screen_height);
    // 描画（active な弾のみ）
    void Render(PrimitiveRenderer& renderer) const;

    // Debug用
    std::size_t CountActive() const noexcept;
    std::size_t Capacity()    const noexcept{ return bullets_.size(); }

private:
    std::vector<Bullet> bullets_;
    std::size_t next_spawn_idx_ = 0;

    // 弾が画面外かどうか(弾の半径を考慮して完全に出たらtrueにする)
    bool IsOutOfScreen(const Bullet& bullet, float screen_width, float screen_height) const;
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BULLET_BULLETSYSTEM_HPP_
