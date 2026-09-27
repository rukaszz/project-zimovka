#ifndef ZIMOVKA_SYSTEMS_SPAWN_ENEMYSPAWNER_HPP_
#define ZIMOVKA_SYSTEMS_SPAWN_ENEMYSPAWNER_HPP_

#include <cstddef>
#include <cstdint>
#include <span>

#include "zimovka/systems/enemy/EnemySystem.hpp"
#include "zimovka/events/EnemySpawnEvent.hpp"

namespace zimovka{
/**
 * @brief 敵出現イベント管理用構造体
 * 
 */
struct EnemySpawnTickResult{
    std::uint32_t spawned      = 0; // 呼び出した数
    std::uint32_t spawn_failed = 0; // 呼び出しに失敗した数
};
 
/**
 * @brief 敵のスポーンを管理するクラス
 * 
 */
class EnemySpawner{
private:
    // 出現イベントはspanでviewとして受け取る
    std::span<const EnemySpawnEvent> events_{};
    std::size_t next_event_index_ = 0;

public:
    // デフォルトコンストラクタ: 空のspanを持つ(UpdatePipelineのメンバ変数として持つために必要)
    EnemySpawner() noexcept = default;
    // span以外で引数が渡ることを防止するためにexplicit
    explicit EnemySpawner(std::span<const EnemySpawnEvent> events);
    
    // 更新関数
    [[nodiscard]]
    EnemySpawnTickResult UpdateTick(
        std::uint64_t stage_tick,
        EnemySystem& enemies
    );
    // インデックスリセット
    void Reset() noexcept;

    // getter
    // 全スポーンイベントを消化したか
    [[nodiscard]]
    bool IsFinished() const noexcept{
        return next_event_index_ >= events_.size();
    }
    // 次のイベントインデックス取得
    [[nodiscard]]
    std::size_t GetNextEventIndex() const noexcept{
        return next_event_index_;
    }
};
} // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_SPAWN_ENEMYSPAWNER_HPP_
