#include <gtest/gtest.h>

#include <algorithm>

#include "zimovka/config/SimulationConfig.hpp"
#include "zimovka/core/Vec2.hpp"
#include "zimovka/systems/enemy/Enemy.hpp"
#include "zimovka/events/EnemyHitEvents.hpp"
#include "zimovka/systems/bullet/Bullet.hpp"
#include "zimovka/systems/bullet/BulletSystem.hpp"
#include "zimovka/systems/collision/CollisionSystem.hpp"
#include "zimovka/systems/enemy/EnemySpawnParams.hpp"
#include "zimovka/systems/enemy/EnemySystem.hpp"
#include "zimovka/systems/player/Player.hpp"

using zimovka::Bullet;
using zimovka::BulletSystem;
using zimovka::CollisionSystem;
using zimovka::Enemy;
using zimovka::EnemyHitEvents;
using zimovka::EnemySpawnParams;
using zimovka::EnemySystem;
using zimovka::SimulationConfig::FIXED_DELTA_SECONDS;
using zimovka::Player;
using zimovka::Vec2;

namespace {

constexpr float  FIXED_DELTA  = FIXED_DELTA_SECONDS;
constexpr float  WORLD_WIDTH  = 960.0f;
constexpr float  WORLD_HEIGHT = 720.0f;

/**
 * @brief 敵出現用ヘルパ関数
 * 
 * @param pos：指定できるように 
 * @param hp 
 * @return EnemySpawnParams 
 */
EnemySpawnParams MakeEnemyParams(Vec2 pos, int hp = 100){
    EnemySpawnParams p;
    p.position       = pos;
    p.velocity       = {0.0f, 0.0f};
    p.render_size    = {32.0f, 32.0f};
    p.hurtbox_offset = {0.0f, 0.0f};
    p.hurtbox_radius = 13.0f;
    p.contact_offset = {0.0f, 0.0f};
    p.contact_radius = 10.0f;
    p.hp             = hp;
    return p;
}

/**
 * @brief active_count_と実際のactiveな要素数を突き合わせるヘルパ
 *
 * CountActive()が管理する内部カウンタと，配列を線形走査して得た実数を比較し
 * active_count_の整合性をTickまたはサイクルをまたいで継続的に確認する
 *
 * @tparam Range  Bullet/Enemyなどactiveフィールドを持つ要素の列
 * @param values 
 * @return std::size_t 
 */
template <class Range>
std::size_t CountActiveByScan(const Range& values){
    return static_cast<std::size_t>(
        // valuesで受け取ったクラスを操作し，activeを調べてtrueの数を計測し返す
        std::count_if(
            values.begin(), 
            values.end(), 
            [](const auto& value){
                return value.active;
            }
        )
    );
}

}   // namespace

// ──────────────────────────────────────────────────────
// 定常負荷試験(Steady)
// ──────────────────────────────────────────────────────
/**
 * @brief 600Tick更新してもactive数・CollisionStatsが安定していることを確認
 *
 * 配置(全オブジェクトvelocity=0):
 *   Player:        position=(900,600), hit_radius=4
 *   Enemy Bullet:  1200発 / 40列×30行グリッド(spacing=18px, 始点(12,12))
 *                  → 最大x=732, 最大y=552→全発画面内にactiveで留まる
 *   Player Bullet: 100発 / 10列×10行(870-915, 480-525)
 *                  → 敵(y=100)までの最短距離≈566px >> hurtbox+radius=16
 *   Enemy:         10体 / y=100, x=0,50,...,450
 *                  → hp=100で撃破されない
 *
 * 全Tick不変条件:
 *   プレイヤーが被弾しない
 *   自機弾が敵に命中しない
 *   enemy_bullets.CountActive()   == 1200
 *   player_bullets.CountActive()  == 100
 *   enemies.CountActive()         == 10
 *   player_vs_enemy_bullet_checks == 1200 (全敵弾を走査)
 *   player_bullet_vs_enemy_checks == 1000 (100発×10体)
 */
TEST(SteadyTest, ActiveCountsAndStatsAreStableFor600Ticks){
    // ──── システム生成 ────
    BulletSystem    enemy_bullets(1200);
    BulletSystem    player_bullets(100);
    EnemySystem     enemies(10);
    CollisionSystem collision_system;

    // Player
    Player player;
    player.position   = {900.0f, 600.0f};
    player.hit_radius = 4.0f;

    // 敵弾配置: 40列×30行グリッド(spacing=18px, 始点(12,12))
    // x: 8, 26, ..., 710 / y: 8, 26, ..., 530
    // 全発画面内(radius=3): 右端710 < 960, 下端530 < 720
    // 左端に寄せてPlayerとの接触を避ける
    for(int row = 0; row < 30; ++row){
        for(int col = 0; col < 40; ++col){
            const float bx = 8.0f + static_cast<float>(col) * 18.0f;
            const float by = 8.0f + static_cast<float>(row) * 18.0f;
            ASSERT_TRUE(enemy_bullets.Spawn({bx, by}, {0.0f, 0.0f}, 3.0f))
                << "enemy_bullet Spawn失敗: row=" << row << " col=" << col;
        }
    }
    ASSERT_EQ(enemy_bullets.CountActive(), 1200u);

    // 自機弾配置: 10列×10行(870-915, 480-525) 
    // Playerと敵の中間付近, どちらとも衝突しない
    for(int row = 0; row < 10; ++row){
        for(int col = 0; col < 10; ++col){
            const float bx = 870.0f + static_cast<float>(col) * 5.0f;
            const float by = 480.0f + static_cast<float>(row) * 5.0f;
            ASSERT_TRUE(player_bullets.Spawn({bx, by}, {0.0f, 0.0f}, 3.0f))
                << "player_bullet Spawn失敗: row=" << row << " col=" << col;
        }
    }
    ASSERT_EQ(player_bullets.CountActive(), 100u);

    // 敵配置: y=100, x=0,50,...,450 (10体) 
    for(int i = 0; i < 10; ++i){
        // 画面内部に収まるように配置
        const Vec2 position{
            50.0f + static_cast<float>(i) * 70.0f,
            100.0f
        };
        ASSERT_TRUE(
            enemies.Spawn(
                MakeEnemyParams(position)
            )
        );
    }
    ASSERT_EQ(enemies.CountActive(), 10u);

    // Tick開始前のメモリアドレスを取得
    const Bullet* enemy_bullet_data  = enemy_bullets.GetBullets().data();
    const Bullet* player_bullet_data = player_bullets.GetBullets().data();
    const Enemy*  enemy_data         = enemies.GetEnemies().data();

    // ──── 600Tick定常負荷試験 ────
    for(std::size_t tick = 0; tick < 600; ++tick){
        collision_system.InitializeStatsAtBeginTick();

        enemy_bullets.Update(FIXED_DELTA, WORLD_WIDTH, WORLD_HEIGHT);
        player_bullets.Update(FIXED_DELTA, WORLD_WIDTH, WORLD_HEIGHT);
        enemies.Update(FIXED_DELTA, WORLD_WIDTH, WORLD_HEIGHT);

        // プレイヤーが被弾しないことを確認
        ASSERT_FALSE(
            collision_system.CheckPlayerHitByBullets(player, enemy_bullets)
        ) << "Tick " << tick << ": プレイヤーが被弾";

        // 自機弾と敵の衝突解決
        const EnemyHitEvents hits =
            collision_system.ResolvePlayerBulletsVsEnemies(player_bullets, enemies);

        ASSERT_EQ(hits.hit_count,  0u) << "Tick " << tick;
        ASSERT_EQ(hits.kill_count, 0u) << "Tick " << tick;

        // active数が変化しないことを確認
        ASSERT_EQ(enemy_bullets.CountActive(),  1200u) << "Tick " << tick;
        ASSERT_EQ(player_bullets.CountActive(), 100u)  << "Tick " << tick;
        ASSERT_EQ(enemies.CountActive(),         10u)  << "Tick " << tick;

        // active数の計測と，実際のactiveな要素数が一致しているか確認
        ASSERT_EQ(
            enemy_bullets.CountActive(),
            CountActiveByScan(enemy_bullets.GetBullets())
        );
        ASSERT_EQ(
            player_bullets.CountActive(),
            CountActiveByScan(player_bullets.GetBullets())
        );
        ASSERT_EQ(
            enemies.CountActive(),
            CountActiveByScan(enemies.GetEnemies())
        );

        // 衝突判定カウントが毎Tick正確であることを確認
        ASSERT_EQ(
            collision_system.GetStats().player_vs_enemy_bullet_checks,
            1200u
        ) << "Tick " << tick;
        ASSERT_EQ(
            collision_system.GetStats().player_bullet_vs_enemy_checks,
            1000u
        ) << "Tick " << tick;
    }

    // vectorの内部ストレージが再確保されていないか調べる
    EXPECT_EQ(
        enemy_bullets.GetBullets().data(),
        enemy_bullet_data
    );
    EXPECT_EQ(
        player_bullets.GetBullets().data(),
        player_bullet_data
    );
    EXPECT_EQ(
        enemies.GetEnemies().data(),
        enemy_data
    );
}
