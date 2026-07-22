#include "zimovka/systems/enemy/EnemySystem.hpp"

#include <cmath>
#include <stdexcept>

#include "zimovka/rendering/Color.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"

namespace zimovka{
/**
 * @brief 暗黙的な型変換を防止したコンストラクタ
 * 
 * @param capacity 
 */
EnemySystem::EnemySystem(std::size_t capacity)
    : next_spawn_index_(0)
{
    // resizeで空要素で連続メモリ確保
    enemies_.resize(capacity);
}
// 生成
bool EnemySystem::Spawn(
    Vec2 position, Vec2 velocity, 
    Vec2 size, std::int32_t hp
)
{
    // 引数チェック(NaNや負の値を検知)
    const bool valid_size = std::isfinite(size.x)
                         && std::isfinite(size.y)
                         && size.x > 0.0f
                         && size.y > 0.0f;
    if(!valid_size){
        return false;
    }
    if(hp <= 0){
        return false;
    }
    // 配列サイズ取得
    const std::size_t array_size = enemies_.size();
    // 満杯なら生成不可で即return
    if(active_count_ >= array_size){
        return false;
    }
    // 配列を走査
    for(std::size_t i = 0; i < array_size; ++i){
        // 次に出現するインデックス設定(0〜MAXの範囲で回る)
        const std::size_t idx = (next_spawn_index_ + i) % array_size;
        // inactiveなenemyを発見したらactiveに
        if(!enemies_[idx].active){
            enemies_[idx].active      = true;
            enemies_[idx].position    = position;
            enemies_[idx].velocity    = velocity;
            enemies_[idx].render_size = size;
            enemies_[idx].hp          = hp;
            // 次のSpawn()では見つけた非活性のenemies_インデックス+1から探す
            next_spawn_index_ = (idx +1) % array_size;
            ++active_count_;
            return true;
        }
    }
    // 全配列を操作してinactiveがゼロ=プール満杯(出現不可)
    return false;
}

/**
 * @brief 初期化用メソッド
 * 
 */
void EnemySystem::Clear() noexcept{
    // 全ての敵をinactive
    for (Enemy& enemy : enemies_) {
        enemy.active = false;
    }
    // count/indexもリセット
    active_count_ = 0;
    next_spawn_index_ = 0;
}

/**
 * @brief 敵の更新関数
 * 
 * @param dt 
 * @param world_width 
 * @param world_height 
 */
void EnemySystem::Update(float dt, float world_width, float world_height){
    // enemies_線形全探索
    for(auto& e : enemies_){
        // inactiveは飛ばす
        if(!e.active){
            continue;
        }
        // 座標更新
        e.position += e.velocity *dt;
        // 画面外へ出たら非活性化
        if(IsOutOfScreen(e, world_width, world_height)){
            e.active = false;
            --active_count_;
        }
    }
}

/**
 * @brief 敵の描画処理
 * 現状は暫定的に矩形で描画する
 * 
 * @param renderer 
 */
void EnemySystem::Render(PrimitiveRenderer& renderer) const{
    // enemies_線形全探索
    for(const auto& e : enemies_){
        // inactiveは飛ばす
        if(!e.active){
            continue;
        }
        // activeな敵は描画
        Color enemy_color{32, 32, 128, 255};
        // enemy.positionは中心座標
        renderer.DrawFilledRect(
            e.position.x - e.render_size.x * 0.5f,
            e.position.y - e.render_size.y * 0.5f,
            e.render_size.x,
            e.render_size.y,
            enemy_color
        );
        Color enemy_center{128, 32, 32, 255};
        renderer.DrawFilledCircle(
            e.position.x, 
            e.position.y,
            static_cast<int>(e.hurtbox_radius),
            enemy_center
        );
    }
}

/**
 * @brief 
 * 
 * @param index 
 * @param damage 
 * @return true 
 * @return false 
 */
EnemyDamageResult EnemySystem::TakeDamage(std::size_t index, std::int32_t damage){
    // indexのチェック(index>=capacityで未定義動作)
    if(index >= enemies_.size()){
        throw std::out_of_range("EnemySystem::TakeDamage index out of range.");
    }
    // damage < 0だと回復してしまうので不正とする
    if(damage <= 0){
        return EnemyDamageResult::InvalidTarget;
    }
    auto& e = enemies_[index];
    // inactiveは処理しない
    if(!e.active){
        return EnemyDamageResult::InvalidTarget;
    }
    // hpを減らす
    e.hp -= damage;
    // hpがゼロ以下になったら非活性
    if(e.hp <= 0){
        e.active = false;
        --active_count_;
        return EnemyDamageResult::Destroyed;
    }
    // ダメージを受けたのでtrue
    return EnemyDamageResult::Damaged;
}

/**
 * @brief 画面外へ出たかを判定する
 * 
 * @param enemy 
 * @param wirld_width 
 * @param world_height 
 * @return true 
 * @return false 
 */
bool EnemySystem::IsOutOfScreen(const Enemy& enemy, float world_width, float world_height) const{
    // 描画サイズの半分を判定の基準にする
    const float half_w = enemy.render_size.x * 0.5f;
    const float half_h = enemy.render_size.y * 0.5f;
    
    // 画面へ消えたことを上下左右を調べる
    return (enemy.position.x + half_w < 0.0f          // 右端が画面左端の外
         || enemy.position.x - half_w > world_width   // 左端が画面右端の外
         || enemy.position.y + half_h < 0.0f          // 下端が画面上端の外
         || enemy.position.y - half_h > world_height  // 上端が画面下端の外
        );
}

}   // namespace zimovka
