#ifndef ZIMOVKA_SYSTEMS_ENEMY_ENEMY_HPP_
#define ZIMOVKA_SYSTEMS_ENEMY_ENEMY_HPP_

#include <cstdint>

#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief 敵のデータ構造(コンポーネント)
 * 
 * 当たり判定(hurtbox)は円で管理し，大型の敵は円の組み合わせで表現する
 */
struct Enemy{
    // 活性/非活性で管理
    bool active = false;
    // ゲーム上の中心座標/速度
    Vec2 position{0, 0};
    Vec2 velocity{0, 0};
    // 描画用サイズ(描画時のみ左上座標で管理する)
    Vec2 render_size{32.0f, 32.0f};

    // 自機弾を受けるhurtbox
    Vec2 hurtbox_offset{};
    float hurtbox_radius = 13.0f;

    // Playerとの接触判定用円
    Vec2 contact_offset{};
    float contact_radius = 10.0f;

    std::int32_t hp = 1;
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_ENEMY_ENEMY_HPP_
