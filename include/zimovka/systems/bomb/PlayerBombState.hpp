#ifndef ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBSTATE_HPP_
#define ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBSTATE_HPP_

#include <cstdint>

namespace zimovka{
/**
 * @brief プレイヤーのボムに関する状態管理用構造体
 * 
 */
struct PlayerBombState{
    std::uint32_t stock = 3;
    std::uint32_t invincible_ticks_remaining = 0;       // ボム発動中の無敵
    std::uint32_t grace_ticks_remaining      = 0;       // 食らいボム受け付け時間
    bool          has_pending_hit            = false;   // 解決待ちの被弾

    // getter
    bool IsInvincible() const noexcept{
        return invincible_ticks_remaining;
    }
    bool InGrace() const noexcept{
        return has_pending_hit;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBSTATE_HPP_
