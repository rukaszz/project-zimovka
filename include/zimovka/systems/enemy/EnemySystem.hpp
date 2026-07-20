#ifndef ZIMOVKA_SYSTEMS_ENEMY_ENEMYSYSTEM_HPP_
#define ZIMOVKA_SYSTEMS_ENEMY_ENEMYSYSTEM_HPP_

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "zimovka/systems/enemy/Enemy.hpp"

namespace zimovka{

// 前方宣言
class PrimitiveRenderer;

/**
 * @brief AoSに基づいて敵を連続メモリで管理するシステム
 * 
 * 敵の生成はinactive→activeの状態遷移であり，オブジェクト生成(ヒープ)は実施しない
 * ゲーム開始時に最大値までメモリを確保する※ゲーム中にヒープ領域は確保しない
 * 
 */
class EnemySystem{
private:
    std::vector<Enemy> enemies_;
    std::size_t active_count_ = 0;
    std::size_t next_spawn_index_ = 0;

public:
    // 暗黙的な型変換を防止したコンストラクタ
    explicit EnemySystem(std::size_t capacity = 32);
    // 生成
    bool Spawn(
        Vec2 posision, 
        Vec2 velocity, 
        Vec2 size, 
        std::int32_t hp
    );
    // 更新
    void Update(float dt, float wirld_width, float world_height);
    // 描画
    void Render(PrimitiveRenderer& renderer) const;
    // ダメージを受ける
    bool TakeDamage(std::size_t index, std::int32_t damage);
    // 画面外判定
    bool IsOutOfScreen(const Enemy& Enemy, float wirld_width, float world_height) const;

    // getter
    std::span<const Enemy> GetEnemies() const noexcept{
        return enemies_;
    }
    std::size_t CountActive() const noexcept;
    std::size_t GetCapacity() const noexcept{
        return enemies_.size();
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_ENEMY_ENEMYSYSTEM_HPP_
