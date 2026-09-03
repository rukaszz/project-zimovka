#ifndef ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBEVENTS_HPP_
#define ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBEVENTS_HPP_

#include <cstddef>

namespace zimovka{
/**
 * @brief プレイヤーのボムに関するイベント用構造体
 * 
 */
struct PlayerBombEvents{
    bool activated     = false; // このTickでボム発動(SE/エフェクトのトリガー)
    bool hit_cancelled = false; // 食らいボムで被弾キャンセル
    bool hit_applied   = false; // 被弾が確定した(リセット処理のトリガー)
    // 敵/弾の情報を保持する
    std::size_t cleared_bullet_count = 0;
    std::size_t enemy_kill_count = 0;
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBEVENTS_HPP_
