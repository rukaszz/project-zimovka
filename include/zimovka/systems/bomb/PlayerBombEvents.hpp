#ifndef ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBEVENTS_HPP_
#define ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBEVENTS_HPP_

namespace zimovka{
/**
 * @brief プレイヤーのボムに関するイベント用構造体
 * 
 */
struct PlayerBombEvents{
    bool activated     = false; // このTickでボム発動(SE/エフェクトのトリガー)
    bool hit_cancelled = false; // 食らいボムで被弾キャンセル
    bool hit_applied   = false; // 被弾が確定した(リセット処理のトリガー)
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_BOMB_PLAYERBOMBEVENTS_HPP_
