#include "zimovka/systems/player/PlayerWeaponSystem.hpp"

#include <stdexcept>

#include "zimovka/input/Action.hpp"
#include "zimovka/core/Vec2.hpp"

namespace zimovka{
/**
 * @brief Construct a new Player Weapon System:: Player Weapon System object
 * 
 * 引数としてPlayerWeaponConfigを受け取り内容をチェックする
 * 
 * @param config 
 */
PlayerWeaponSystem::PlayerWeaponSystem(
    PlayerWeaponConfig config
) : config_(config)
{
    // 引数のconfigを分解して内容をチェック
    if(config_.max_ammo == 0){
        throw std::invalid_argument(
            "PlayerWeaponConfig::max_ammo must be positive. "
        );
    }
    if(config_.reload_duration_ticks == 0){
        throw std::invalid_argument(
            "PlayerWeaponConfig::reload_duration_ticks must be positive. "
        );
    }
    if(config_.bullet_speed <= 0.0f){
        throw std::invalid_argument(
            "PlayerWeaponConfig::bullet_speed must be positive. "
        );
    }
    if(config_.bullet_radius <= 0.0f){
        throw std::invalid_argument(
            "PlayerWeaponConfig::bullet_radius must be positive. "
        );
    }
    // 初期化
    Reset();
}

/**
 * @brief 状態(state_)のリセット
 * 
 */
void PlayerWeaponSystem::Reset() noexcept{
    state_.ammo = config_.max_ammo;
    state_.cooldown_ticks_remaining = 0;
    state_.reload_ticks_remaining   = 0;
}

/**
 * @brief 武器のリロードで呼ばれる
 * 
 * @param events 
 */
void PlayerWeaponSystem::StartReload(WeaponTickEvents& events) noexcept{
    // リロード時間設定
    state_.reload_ticks_remaining = config_.reload_duration_ticks;
    events.reload_started = true;   // 参照しているのでtrueが設定される
}

/**
 * @brief PlayerWeaponSystemの状態を，Tick(ゲームループ単位)で更新する
 * 
 * Weaponの状況はWeaponTikcEventsで管理する
 * リロードはイベントを発行するだけ
 * 
 * @param input 
 * @param player 
 * @param player_bullets 
 * @return WeaponTickEvents 
 */
WeaponTickEvents PlayerWeaponSystem::UpdateTick(
    const InputState& input, 
    const Player& player, 
    BulletSystem& player_bullets
)
{
    // イベントの状態保持用変数
    WeaponTickEvents events{};
    // ────────────────────────────────
    // 最初に射撃クールダウン/リロードの状態のチェック    
    // ────────────────────────────────
    // クールダウン中でもリロードは進行する
    if(state_.cooldown_ticks_remaining > 0){
        --state_.cooldown_ticks_remaining;
    }
    // リロード中は射撃できない
    if(state_.reload_ticks_remaining > 0){
        --state_.reload_ticks_remaining;
        // リロード時間が0になったら
        if(state_.reload_ticks_remaining == 0){
            state_.ammo = config_.max_ammo; // 残弾MAX
            events.reload_completed = true; // リロード完了
        }
        return events;
    }
    // 外的要因で残弾ゼロになった場合もreload開始
    if(state_.ammo == 0){
        StartReload(events);
        return events;
    }
    // ────────────────────────────────
    // 射撃に関する確認や設定    
    // ────────────────────────────────
    // Z押した瞬間だけ発射(押しっぱなしでは撃たない)
    if(!input.IsPressed(Action::Shoot)){
        return events;
    }
    // クールダウン中は撃てない
    if(state_.cooldown_ticks_remaining > 0){
        return events;
    }
    // 発射位置設定
    const Vec2 muzzle_position{
        player.position.x + config_.muzzle_offset.x, 
        player.position.y - player.height * 0.5f   // 中心位置くらいから発射
            + config_.muzzle_offset.y
    };
    // 弾の発生
    // とりあえず別プールでプレイヤー弾を管理している
    const bool fired = player_bullets.Spawn(
        muzzle_position, 
        Vec2{0.0f, -config_.bullet_speed}, 
        config_.bullet_radius, 
        config_.bullet_color
    );
    // 発射できたか
    if(!fired){
        events.spawn_failed = true;
        return events;
    }
    // ────────────────────────────────
    // 発射処理    
    // ────────────────────────────────
    // 残弾-1
    --state_.ammo;
    // クールダウン設定
    state_.cooldown_ticks_remaining = config_.shot_cooldown_ticks;
    // 発射した
    events.shot_fired = true;
    // 残弾0ならリロード
    if(state_.ammo == 0){
        StartReload(events);
    }

    return events;
}

}   // namespace zimovka
