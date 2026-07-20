#ifndef ZIMOVKA_SYSTEMS_PLAYER_PLAYERWEAPONCONFIG_HPP_
#define ZIMOVKA_SYSTEMS_PLAYER_PLAYERWEAPONCONFIG_HPP_

#include <cstdint>

#include "zimovka/core/Vec2.hpp"
#include "zimovka/rendering/Color.hpp"

namespace zimovka{
/**
 * @brief プレイヤー弾や発射処理に関する設定
 * 
 * 静的な情報はこちらで管理する
 */
struct PlayerWeaponConfig{
    // 最大弾数
    std::uint32_t max_ammo = 6;
    // クールダウンなどの時間はTick(ゲームループ基準の間隔)で管理する
    // 連続射撃抑制用のクールダウン
    std::uint32_t shot_cooldown_ticks = 8;
    // 球切れから装填完了までの固定時間
    std::uint32_t reload_duration_ticks = 90;
    // 弾の設定
    float bullet_speed = 720.0f;
    float bullet_radius = 3.0f;
    Color bullet_color{120, 220, 255, 255};
    // 弾発射場所調整用のオフセット
    Vec2 muzzle_offset{0.0f, 0.0f};
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_PLAYER_PLAYERWEAPONCONFIG_HPP_
