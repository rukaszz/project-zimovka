#include <gtest/gtest.h>

#include <stdexcept>

#include "zimovka/events/PlayerWeaponTickEvents.hpp"
#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/player/Player.hpp"
#include "zimovka/systems/player/PlayerWeaponConfig.hpp"
#include "zimovka/systems/player/PlayerWeaponState.hpp"
#include "zimovka/systems/player/PlayerWeaponSystem.hpp"

using zimovka::Action;
using zimovka::BulletSystem;
using zimovka::InputState;
using zimovka::Player;
using zimovka::PlayerWeaponConfig;
using zimovka::PlayerWeaponSystem;
using zimovka::WeaponTickEvents;

// ヘルパ関数
namespace{
/**
 * @brief このTickで初めて押した(pressed + held)判定を作る
 * 
 * @return InputState 
 */
InputState ShootPressed(){
    InputState s;
    s.SetPressed(Action::Shoot);
    s.SetHeld(Action::Shoot, true);
    return s;
}

/**
 * @brief 押しっぱなし(heldのみでpressedではない)判定を作る
 * 
 * @return InputState 
 */
InputState ShootHeld(){
    InputState s;
    s.SetHeld(Action::Shoot, true);
    return s;
}

/**
 * @brief 空のInputState生成
 * 
 * @return InputState 
 */
InputState NoInput(){
    return InputState{};
}

/**
 * @brief テストで用いる汎用的なPlayerを作る
 * 
 * @return Player 
 */
Player TestPlayer(){
    Player p;
    p.position = {480.0f, 576.0f};
    p.width    = 24.0f;
    p.height   = 24.0f;
    return p;
}

/**
 * @brief ammo=3, cooldown=2tick, reload=5tickのPWConfigを作る
 * 多段シナリオ用
 * 
 * @return PlayerWeaponConfig 
 */
PlayerWeaponConfig SmallConfig(){
    PlayerWeaponConfig c;
    c.max_ammo              = 3;
    c.shot_cooldown_ticks   = 2;
    c.reload_duration_ticks = 5;
    c.bullet_speed  = 720.0f;
    c.bullet_radius = 3.0f;
    return c;
}

/**
 * @brief ammo=1, cooldown=0, reload=3tickのConfig生成
 * 最小シナリオ用
 * 
 * @return PlayerWeaponConfig 
 */
PlayerWeaponConfig MinimalConfig(){
    PlayerWeaponConfig c;
    c.max_ammo              = 1;
    c.shot_cooldown_ticks   = 0;
    c.reload_duration_ticks = 3;
    c.bullet_speed  = 720.0f;
    c.bullet_radius = 3.0f;
    return c;
}

/**
 * @brief N tickの空の更新を行う
 * BulletSystem::Updateは呼ばないので弾は動かない
 * 
 * @param pws 
 * @param n 
 * @param bs 
 * @param p 
 */
void AdvanceTicks(PlayerWeaponSystem& pws, int n, BulletSystem& bs, const Player& p){
    for(int i = 0; i < n; ++i){
        pws.UpdateTick(NoInput(), p, bs);
    }
}

}   // namespace

// ──────────────────────────────────────────────────────
// コンストラクタ / 初期状態
// ──────────────────────────────────────────────────────
/**
 * @brief 初期ammoがmax_ammoと等しいことを確認
 * 
 */
TEST(PlayerWeaponSystemTest, InitialState_AmmoEqualsMaxAmmo){
    PlayerWeaponSystem pws;
    EXPECT_EQ(pws.GetState().ammo, pws.GetConfig().max_ammo);
}

/**
 * @brief 初期状態でクールダウンがないことを確認
 * 
 */
TEST(PlayerWeaponSystemTest, InitialState_NoCooldown){
    PlayerWeaponSystem pws;
    EXPECT_EQ(pws.GetState().cooldown_ticks_remaining, 0u);
}

/**
 * @brief 初期状態で発射可能(IsReadyToFire)であることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, InitialState_IsReadyToFire){
    PlayerWeaponSystem pws;
    EXPECT_TRUE(pws.GetState().IsReadyToFire());
}

/**
 * @brief 初期状態でリロード中でないことを確認
 * 
 */
TEST(PlayerWeaponSystemTest, InitialState_NotReloading){
    PlayerWeaponSystem pws;
    EXPECT_FALSE(pws.GetState().IsReloading());
}

/**
 * @brief max_ammo=0のPWConfigでは例外が投げられることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, InvalidConfig_ZeroAmmo_Throws){
    PlayerWeaponConfig c;
    c.max_ammo = 0;
    EXPECT_THROW((PlayerWeaponSystem{c}), std::invalid_argument);
}

/**
 * @brief reload_duration_ticks = 0のPWConfigでは例外が投げられることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, InvalidConfig_ZeroReloadDuration_Throws){
    PlayerWeaponConfig c;
    c.reload_duration_ticks = 0;
    EXPECT_THROW((PlayerWeaponSystem{c}), std::invalid_argument);
}

/**
 * @brief bullet_speed = 0のPWConfigでは例外が投げられることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, InvalidConfig_ZeroBulletSpeed_Throws){
    PlayerWeaponConfig c;
    c.bullet_speed = 0.0f;
    EXPECT_THROW((PlayerWeaponSystem{c}), std::invalid_argument);
}

/**
 * @brief bullet_speed < 0のPWConfigでは例外が投げられることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, InvalidConfig_NegativeBulletSpeed_Throws){
    PlayerWeaponConfig c;
    c.bullet_speed = -1.0f;
    EXPECT_THROW((PlayerWeaponSystem{c}), std::invalid_argument);
}

/**
 * @brief bullet_radius = 0のPWConfigでは例外が投げられることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, InvalidConfig_ZeroBulletRadius_Throws){
    PlayerWeaponConfig c;
    c.bullet_radius = 0.0f;
    EXPECT_THROW((PlayerWeaponSystem{c}), std::invalid_argument);
}

// ──────────────────────────────────────────────────────
// Reset
// ──────────────────────────────────────────────────────
/**
 * @brief Reset()後にammoがmax_ammoに戻ることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, Reset_RestoresAmmoToMax){
    PlayerWeaponSystem pws(SmallConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 1発撃って ammo を減らす
    ASSERT_LT(pws.GetState().ammo, pws.GetConfig().max_ammo);
    pws.Reset();
    EXPECT_EQ(pws.GetState().ammo, pws.GetConfig().max_ammo);
}

/**
 * @brief Reset()後にクールダウンがクリアされることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, Reset_ClearsCooldown){
    PlayerWeaponSystem pws(SmallConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 発射 → cooldown = 2
    ASSERT_GT(pws.GetState().cooldown_ticks_remaining, 0u);
    pws.Reset();
    EXPECT_EQ(pws.GetState().cooldown_ticks_remaining, 0u);
}

/**
 * @brief Reset()後にリロードがクリアされることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, Reset_ClearsReload){
    PlayerWeaponSystem pws(MinimalConfig());    // ammo = 1 → 1発発射でリロード
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 発射 → リロード開始
    ASSERT_TRUE(pws.GetState().IsReloading());
    pws.Reset();
    EXPECT_FALSE(pws.GetState().IsReloading());
}

/**
 * @brief Reset()後にIsReadyToFire()がtrueになることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, Reset_StateIsReadyToFire){
    PlayerWeaponSystem pws(SmallConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);
    pws.Reset();    // 線条体が戻る
    EXPECT_TRUE(pws.GetState().IsReadyToFire());
}

// ──────────────────────────────────────────────────────
// PlayerWeaponStateクエリメソッド
// ──────────────────────────────────────────────────────
/**
 * @brief クールダウン中はIsReadyToFire()がfalseになることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, IsReadyToFire_FalseWhenCooldown){
    PlayerWeaponSystem pws(SmallConfig());      // cooldown = 2
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 発射 → cooldown=2
    EXPECT_FALSE(pws.GetState().IsReadyToFire());
}

/**
 * @brief リロード中はIsReadyToFire()がfalseになることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, IsReadyToFire_FalseWhenReloading){
    PlayerWeaponSystem pws(MinimalConfig());    // ammo=1
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 発射 → ammo=0 → リロード
    EXPECT_FALSE(pws.GetState().IsReadyToFire());
}

/**
 * @brief ammoが0になるとIsReloading()がtrueになることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, IsReloading_TrueAfterAmmoZero){
    PlayerWeaponSystem pws(MinimalConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);
    EXPECT_TRUE(pws.GetState().IsReloading());
}

// ──────────────────────────────────────────────────────
// 発射: ボタン押下状態
// ──────────────────────────────────────────────────────
/**
 * @brief IsPressedのときshot_firedイベントが発生することを確認
 * 
 */
TEST(PlayerWeaponSystemTest, ShootPressed_ShotFiredEvent){
    PlayerWeaponSystem pws;
    BulletSystem bs(10);
    Player p = TestPlayer();
    const auto ev = pws.UpdateTick(ShootPressed(), p, bs);
    EXPECT_TRUE(ev.shot_fired);
}

/**
 * @brief IsHeldのみ(押しっぱなし)では発射しないことを確認
 *
 * ※zimovkaの仕様として，ボタンを押した瞬間のみ発射し連射はしないため
 */
TEST(PlayerWeaponSystemTest, ShootHeldOnly_NoFire){
    PlayerWeaponSystem pws;
    BulletSystem bs(10);
    Player p = TestPlayer();
    const auto ev = pws.UpdateTick(ShootHeld(), p, bs);
    EXPECT_FALSE(ev.shot_fired);
}

/**
 * @brief 入力なしでは発射しないことを確認
 * 
 */
TEST(PlayerWeaponSystemTest, NoInput_NoFire){
    PlayerWeaponSystem pws;
    BulletSystem bs(10);
    Player p = TestPlayer();
    const auto ev = pws.UpdateTick(NoInput(), p, bs);
    EXPECT_FALSE(ev.shot_fired);
}

/**
 * @brief 発射でammoが1減ることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, ShootPressed_DecrementsAmmo){
    PlayerWeaponSystem pws;
    BulletSystem bs(10);
    Player p = TestPlayer();
    // 発射前の残弾
    const std::uint32_t ammo_before = pws.GetState().ammo;
    pws.UpdateTick(ShootPressed(), p, bs);
    EXPECT_EQ(pws.GetState().ammo, ammo_before - 1u);
}

/**
 * @brief 発射でplayer_bulletsに弾が1発生成されることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, ShootPressed_SpawnsBulletInPool){
    PlayerWeaponSystem pws;
    BulletSystem bs(10);
    Player p = TestPlayer();
    // この時点ではゼロ
    ASSERT_EQ(bs.CountActive(), 0u);
    pws.UpdateTick(ShootPressed(), p, bs);
    EXPECT_EQ(bs.CountActive(), 1u);
}

/**
 * @brief 生成された弾の位置がmuzzle_positionと一致することを確認
 *
 * muzzle_x = player.position.x + muzzle_offset.x
 * muzzle_y = player.position.y - player.height * 0.5f + muzzle_offset.y
 */
TEST(PlayerWeaponSystemTest, ShootPressed_BulletAtMuzzlePosition){
    PlayerWeaponSystem pws;    // muzzle_offset = {0, 0}
    BulletSystem bs(10);
    Player p = TestPlayer();   // position={480, 576}, height=24
    pws.UpdateTick(ShootPressed(), p, bs);
    const auto& bullet = bs.GetBullets()[0];
    EXPECT_TRUE(bullet.active);
    EXPECT_FLOAT_EQ(bullet.position.x, p.position.x);
    EXPECT_FLOAT_EQ(bullet.position.y, p.position.y - p.height * 0.5f);
}

/**
 * @brief 生成された弾の速度が上方向(y軸負方向)であることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, ShootPressed_BulletVelocityIsUpward){
    PlayerWeaponSystem pws;
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);
    const auto& bullet = bs.GetBullets()[0];
    EXPECT_FLOAT_EQ(bullet.velocity.x, 0.0f);
    EXPECT_FLOAT_EQ(bullet.velocity.y, -pws.GetConfig().bullet_speed);
}

/**
 * @brief 発射後にcooldown_ticks_remainingが設定されることを確認
 */
TEST(PlayerWeaponSystemTest, ShootPressed_SetsCooldown){
    PlayerWeaponSystem pws(SmallConfig());      // shot_cooldown_ticks = 2
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);
    EXPECT_EQ(pws.GetState().cooldown_ticks_remaining, SmallConfig().shot_cooldown_ticks);
}

/**
 * @brief muzzle_offsetが弾の生成位置に反映されることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, MuzzleOffset_AffectsBulletPosition){
    PlayerWeaponConfig c = MinimalConfig();
    c.muzzle_offset = {10.0f, -5.0f};
    PlayerWeaponSystem pws(c);
    BulletSystem bs(10);
    Player p = TestPlayer();   // position={480, 576}, height=24
    pws.UpdateTick(ShootPressed(), p, bs);
    const auto& bullet = bs.GetBullets()[0];
    EXPECT_FLOAT_EQ(bullet.position.x, p.position.x + 10.0f);
    EXPECT_FLOAT_EQ(bullet.position.y, p.position.y - p.height * 0.5f - 5.0f);
}

// ──────────────────────────────────────────────────────
// クールダウン
// ──────────────────────────────────────────────────────
/**
 * @brief 発射直後の再発射試行がcooldownによって阻止されることを確認
 *
 * SmallConfig: cooldown = 2 → 発射後の次tick1回目ではまだ撃てない
 */
TEST(PlayerWeaponSystemTest, Cooldown_BlocksImmediateRefire){
    PlayerWeaponSystem pws(SmallConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 発射 → cooldown=2
    const auto ev = pws.UpdateTick(ShootPressed(), p, bs); // cooldown 2→1, 撃てない
    EXPECT_FALSE(ev.shot_fired);
}

/**
 * @brief cooldownが毎tick 1ずつ減ることを確認
 *
 * 発射後 → cooldown = 2, 1tick後 → cooldown = 1
 */
TEST(PlayerWeaponSystemTest, Cooldown_DecrementsEachTick){
    PlayerWeaponSystem pws(SmallConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // cooldown=2
    pws.UpdateTick(NoInput(), p, bs);           // cooldown 2→1
    EXPECT_EQ(pws.GetState().cooldown_ticks_remaining, 1u);
}

/**
 * @brief cooldown経過後に再発射できることを確認
 *
 * SmallConfig(cooldown=2): 1tick待ち後に発射tickでcooldown 1→0となるので撃てる
 */
TEST(PlayerWeaponSystemTest, Cooldown_ExpiredAllowsRefire){
    PlayerWeaponSystem pws(SmallConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 発射 → cooldown=2
    pws.UpdateTick(NoInput(), p, bs);           // cooldown 2→1
    const auto ev = pws.UpdateTick(ShootPressed(), p, bs); // cooldown 1→0, 発射可
    EXPECT_TRUE(ev.shot_fired);
}

// ──────────────────────────────────────────────────────
// リロード
// ──────────────────────────────────────────────────────
/**
 * @brief 最後の弾を撃ったときreload_startedイベントが発生することを確認
 * 
 */
TEST(PlayerWeaponSystemTest, LastShot_StartsReload){
    PlayerWeaponSystem pws(MinimalConfig());    // ammo=1
    BulletSystem bs(10);
    Player p = TestPlayer();
    const auto ev = pws.UpdateTick(ShootPressed(), p, bs);
    EXPECT_TRUE(ev.shot_fired);
    EXPECT_TRUE(ev.reload_started);
}

/**
 * @brief リロード中はShootを押しても発射しないことを確認
 * 
 */
TEST(PlayerWeaponSystemTest, DuringReload_ShootPressed_NoFire){
    PlayerWeaponSystem pws(MinimalConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 発射 → リロード開始
    const auto ev = pws.UpdateTick(ShootPressed(), p, bs); // リロード中
    EXPECT_FALSE(ev.shot_fired);
}

/**
 * @brief リロード中は完了以外のイベントが発生しないことを確認
 *
 * reload_completed以外のフラグが立たない(完了前tick)
 */
TEST(PlayerWeaponSystemTest, DuringReload_NoEventsExceptCompletion){
    PlayerWeaponSystem pws(MinimalConfig());    // reload=3
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // リロード開始
    // reload 3→2 の tick (完了前)
    const auto ev = pws.UpdateTick(ShootPressed(), p, bs);
    EXPECT_FALSE(ev.shot_fired);
    EXPECT_FALSE(ev.reload_started);
    EXPECT_FALSE(ev.reload_completed);
    EXPECT_FALSE(ev.spawn_failed);
}

/**
 * @brief reload_ticks_remainingが毎tick減ることを確認
 *
 * 発射後 → reload=3, 1tick後 → reload=2
 */
TEST(PlayerWeaponSystemTest, ReloadTicks_DecreaseEachTick){
    PlayerWeaponSystem pws(MinimalConfig());    // reload=3
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // reload=3
    pws.UpdateTick(NoInput(), p, bs);           // reload 3→2
    EXPECT_EQ(pws.GetState().reload_ticks_remaining, 2u);
}

/**
 * @brief リロード完了後にammoがmax_ammoに戻ることを確認
 *
 * MinimalConfig: reload=3 → 3tick後に完了
 */
TEST(PlayerWeaponSystemTest, ReloadComplete_RestoresAmmoToMax){
    PlayerWeaponSystem pws(MinimalConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // ammo=0, reload=3
    AdvanceTicks(pws, 3, bs, p);                // 3tick後: reload完了
    EXPECT_EQ(pws.GetState().ammo, pws.GetConfig().max_ammo);
}

/**
 * @brief リロード完了tickにreload_completedイベントが発生することを確認
 * 
 */
TEST(PlayerWeaponSystemTest, ReloadComplete_SetsEvent){
    PlayerWeaponSystem pws(MinimalConfig());    // reload=3
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // reload開始
    AdvanceTicks(pws, 2, bs, p);               // reload 3→2→1
    const auto ev = pws.UpdateTick(NoInput(), p, bs); // reload 1→0(完了)
    EXPECT_TRUE(ev.reload_completed);
    EXPECT_FALSE(ev.reload_started);
}

/**
 * @brief リロード完了後に再発射できることを確認
 * 
 */
TEST(PlayerWeaponSystemTest, AfterReload_CanFireAgain){
    PlayerWeaponSystem pws(MinimalConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 発射 → リロード
    AdvanceTicks(pws, 3, bs, p);                // リロード完了
    const auto ev = pws.UpdateTick(ShootPressed(), p, bs);
    EXPECT_TRUE(ev.shot_fired);
}

/**
 * @brief 複数発の射撃でammoが1発ごとに減ることを確認
 *
 * SmallConfig: cooldown=2 → 1tick待ちの後に再発射可
 * 発射1: ammo 3→2
 * 発射2: ammo 2→1 (cooldown 2→1, 0 で発射)
 */
TEST(PlayerWeaponSystemTest, MultiShot_EachShotDecrementsAmmo){
    PlayerWeaponSystem pws(SmallConfig());
    BulletSystem bs(20);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // ammo 3→2, cooldown=2
    EXPECT_EQ(pws.GetState().ammo, 2u);
    pws.UpdateTick(NoInput(), p, bs);           // cooldown 2→1
    pws.UpdateTick(ShootPressed(), p, bs);      // cooldown 1→0, ammo 2→1
    EXPECT_EQ(pws.GetState().ammo, 1u);
}

// ──────────────────────────────────────────────────────
// spawn_failed (プール満杯)
// ──────────────────────────────────────────────────────
/**
 * @brief 弾プール満杯のときspawn_failedイベントが発生することを確認
 * 
 */
TEST(PlayerWeaponSystemTest, PoolFull_SpawnFailed){
    PlayerWeaponSystem pws;                     // デフォルト PWConfig
    BulletSystem bs(1);                         // capacity=1
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 1発でプール満杯
    ASSERT_EQ(bs.CountActive(), 1u);
    // クールダウン経過後に再発射 → プール満杯でspawn失敗
    AdvanceTicks(pws, static_cast<int>(pws.GetConfig().shot_cooldown_ticks), bs, p);
    const auto ev = pws.UpdateTick(ShootPressed(), p, bs);
    EXPECT_TRUE(ev.spawn_failed);
    EXPECT_FALSE(ev.shot_fired);
}

/**
 * @brief spawn_failed時にammoが消費されないことを確認
 * 
 */
TEST(PlayerWeaponSystemTest, SpawnFailed_AmmoNotConsumed){
    PlayerWeaponSystem pws;
    BulletSystem bs(1);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 1発目 → プール満杯
    const std::uint32_t ammo_before = pws.GetState().ammo;
    AdvanceTicks(pws, static_cast<int>(pws.GetConfig().shot_cooldown_ticks), bs, p);
    pws.UpdateTick(ShootPressed(), p, bs);      // spawn失敗
    EXPECT_EQ(pws.GetState().ammo, ammo_before);
}

/**
 * @brief spawn_failed時にクールダウンが設定されないことを確認
 *
 * 弾が実際には発射されていないのでクールダウンは発生しない
 */
TEST(PlayerWeaponSystemTest, SpawnFailed_CooldownNotSet){
    PlayerWeaponSystem pws;
    BulletSystem bs(1);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);      // 1発目 → cooldown=8
    // クールダウンをクリアする
    AdvanceTicks(pws, static_cast<int>(pws.GetConfig().shot_cooldown_ticks), bs, p);
    ASSERT_EQ(pws.GetState().cooldown_ticks_remaining, 0u);
    pws.UpdateTick(ShootPressed(), p, bs);      // spawn失敗
    EXPECT_EQ(pws.GetState().cooldown_ticks_remaining, 0u);
}

// ──────────────────────────────────────────────────────
// getter
// ──────────────────────────────────────────────────────
/**
 * @brief GetConfig()が設定したPWConfigを返すことを確認
 * 
 */
TEST(PlayerWeaponSystemTest, GetConfig_ReturnsConfig){
    PlayerWeaponSystem pws(SmallConfig());
    EXPECT_EQ(pws.GetConfig().max_ammo,              SmallConfig().max_ammo);
    EXPECT_EQ(pws.GetConfig().shot_cooldown_ticks,   SmallConfig().shot_cooldown_ticks);
    EXPECT_EQ(pws.GetConfig().reload_duration_ticks, SmallConfig().reload_duration_ticks);
}

/**
 * @brief GetState() が現在の状態を返すことを確認
 * 
 */
TEST(PlayerWeaponSystemTest, GetState_ReturnsCurrentState){
    PlayerWeaponSystem pws(SmallConfig());
    BulletSystem bs(10);
    Player p = TestPlayer();
    pws.UpdateTick(ShootPressed(), p, bs);
    const auto& s = pws.GetState();
    EXPECT_EQ(s.ammo, SmallConfig().max_ammo - 1u);
    EXPECT_EQ(s.cooldown_ticks_remaining, SmallConfig().shot_cooldown_ticks);
}
