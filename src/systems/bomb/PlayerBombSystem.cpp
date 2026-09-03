#include "zimovka/systems/bomb/PlayerBombSystem.hpp"

#include <cassert>

#include "zimovka/input/Action.hpp"
#include "zimovka/systems/bomb/PlayerBombConfig.hpp"

namespace zimovka{
/**
 * @brief PlayerBombSystemの状態を初期値に戻す
 *
 */
void PlayerBombSystem::Reset() noexcept{
    state_ = {};
}

/**
 * @brief ボム発動の共通処理
 * ボムを消費し，無敵+弾クリア+敵全ダメージを実施する
 *
 * @param enemy_bullets
 * @param enemy_system
 */
std::size_t PlayerBombSystem::Activate(
    BulletSystem& enemy_bullets,
    EnemySystem&  enemy_system
) noexcept
{
    // 入力チェック(stock>0はチェックしているが契約としてチェック)
    assert(state_.stock > 0);
    // 敵の撃破数計測用
    std::size_t kill_count = 0;
    // ボム消費
    --state_.stock;
    // 無敵時間開始
    state_.invincible_ticks_remaining = PlayerBombConfig::INVINCIBLE_TICKS;
    // 敵弾を全消去
    enemy_bullets.Clear();
    // 全活性敵にダメージ
    const auto enemies = enemy_system.GetEnemies();
    for(std::size_t i = 0; i < enemies.size(); ++i){
        if(enemies[i].active){
            // ダメージを与えた結果を取得
            const auto result = 
                enemy_system.TakeDamage(i, PlayerBombConfig::BOMB_DAMAGE);
            if(result == EnemyDamageResult::Destroyed){
                ++kill_count;
            }
        }
    }
    return kill_count;
}

/**
 * @brief ボムシステムの更新
 * 衝突検出後に呼ぶ想定(UpdatePipeline::ResolveCollisionsの後)
 *
 * 状態遷移(A〜Cで判定していく):
 *   A. 無敵中        → tick消費，被弾無視
 *   B. 食らいボム中  → ボム入力でキャンセル，タイムアウトで被弾確定
 *   C. 通常          → 新規被弾 or ボム入力を処理
 *
 * @param input
 * @param player_hit    衝突検出の結果(ボム解決前)
 * @param enemy_bullets 敵弾プール(ボム発動が確定したらClear)
 * @param enemy_system  敵システム(ボム発動時にダメージを与える)
 * @return PlayerBombEvents
 */
PlayerBombEvents PlayerBombSystem::UpdateTick(
    const InputState& input,
    bool player_hit,
    BulletSystem& enemy_bullets,
    EnemySystem& enemy_system
)
{
    PlayerBombEvents events{};
    
    // ボム入力の確認
    const bool bomb_pressed = input.IsPressed(Action::Bomb);

    // A. 無敵中
    if(state_.IsInvincible()){
        --state_.invincible_ticks_remaining;    // ボムによる無敵時間消費
        return events;
    }

    // B. 食らいボム受付中
    if(state_.HasPendingHit()){
        // ボム入力あり かつ ボムあり → 被弾キャンセル
        if(bomb_pressed && state_.stock > 0){
            // ボム実行直前のactive数を取得
            events.cleared_bullet_count = enemy_bullets.CountActive();
            // ボムによる一掃
            events.enemy_kill_count = Activate(enemy_bullets, enemy_system);
            state_.has_pending_hit       = false;
            state_.grace_ticks_remaining = 0;
            events.activated             = true;
            events.hit_cancelled         = true;
        } else {
            // ボム入力なし → 食らいボム受け付けタイマー消費
            --state_.grace_ticks_remaining;
            if(state_.grace_ticks_remaining == 0){
                state_.has_pending_hit = false;
                events.hit_applied     = true;
            }
        }
        return events;
    }

    // C. 通常状態
    // プレイヤー被弾
    if(player_hit){
        // ボム入力あり かつ ボム残あり
        if(bomb_pressed && state_.stock > 0){
            // 同Tick中なら即時被弾をキャンセル
            // ボム実行直前のactive数を取得
            events.cleared_bullet_count = enemy_bullets.CountActive();
            // ボムによる一掃
            events.enemy_kill_count = Activate(enemy_bullets, enemy_system);
            events.activated     = true;
            events.hit_cancelled = true;
        } else {
            // 食らいボム受付開始
            state_.has_pending_hit       = true;
            state_.grace_ticks_remaining = PlayerBombConfig::GRACE_TICKS;
        }
    // プレイヤー被弾中ではない通常のボム入力
    }else if(bomb_pressed && state_.stock > 0){
        // 通常ボム発動
        // ボム実行直前のactive数を取得
        events.cleared_bullet_count = enemy_bullets.CountActive();
        // ボムによる一掃
        events.enemy_kill_count = Activate(enemy_bullets, enemy_system);
        events.activated = true;
    }

    return events;
}

}   // namespace zimovka
