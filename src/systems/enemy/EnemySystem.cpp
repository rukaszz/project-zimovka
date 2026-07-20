#include "zimovka/systems/enemy/EnemySystem.hpp"

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
    Vec2 posision, Vec2 velocity, 
    Vec2 size, std::int32_t hp
)
{
    // 引数チェック
    if(size.IsZero()){
        return false;
    }
    if(hp == 0){
        return false;
    }
    // 配列サイズ取得
    const std::size_t array_size = enemies_.size();
    // 配列を走査
    for(std::size_t i = 0; i < array_size; ++i){
        // 次に出現するインデックス設定(0〜MAXの範囲で回る)
        const std::size_t idx = (next_spawn_index_ + i) % array_size;
        // inactiveなenemyを発見したらactiveに
        if(!enemies_[idx].active){
            enemies_[idx].active   = true;
            enemies_[idx].posotion = posision;
            enemies_[idx].velocity = velocity;
            enemies_[idx].size     = size;
            enemies_[idx].hp       = hp;
            // 次のSpawn()では見つけた非活性のenemies_インデックス+1から探す
            next_spawn_index_ = (idx +1) % array_size;
            return true;
        }
    }
    // 全配列を操作してinactiveがゼロ=プール満杯(出現不可)
    return false;
}
// 更新
void EnemySystem::Update(float dt, float world_width, float world_height){
    // enemies_線形全探索
    for(auto& e : enemies_){
        // inactiveは飛ばす
        if(!e.active){
            continue;
        }
        // 座標更新
        e.posotion += e.velocity *dt;
        // 画面外へ出たら非活性化
        if(IsOutOfScreen(e, world_width, world_height)){
            e.active = false;
        }
    }
}
// 描画
void EnemySystem::Render(PrimitiveRenderer& renderer) const{
    // enemies_線形全探索
    for(const auto& e : enemies_){
        if(!e.active){
            continue;
        }
        // activeな敵は描画
        Color enemy_color{128, 128, 32, 255};
        renderer.DrawFilledRect(
            e.posotion.x, 
            e.posotion.y,
            e.posotion.x + e.size.x, 
            e.posotion.y + e.size.y, 
            enemy_color
        );
    }
}
// ダメージを受ける
bool EnemySystem::TakeDamage(std::size_t index, std::int32_t damage){

}

bool IsOutOfScreen(const Enemy& Enemy, float wirld_width, float world_height) const{

}


/**
 * @brief 現在活性状態の敵を数える
 * 
 * @return std::size_t 
 */
std::size_t EnemySystem::CountActive() const noexcept{
    // 戻り値
    std::size_t count = 0;
    for(const auto& e : enemies_){
        // 単純な計測
        if(e.active){
            ++count;
        }
    }
    return count;
}
}   // namespace zimovka
