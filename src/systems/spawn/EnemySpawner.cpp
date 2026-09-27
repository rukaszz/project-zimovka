#include "zimovka/systems/spawn/EnemySpawner.hpp"

#include <stdexcept>

namespace zimovka{

/**
 * @brief コンストラクタ
 *
 * @param events スポーンイベント配列のView(ライフタイムは呼び出し元が保証する)
 */
EnemySpawner::EnemySpawner(std::span<const EnemySpawnEvent> events)
    : events_{events}
{
    // EnemySpawnEventがtickの昇順でソートされているかチェック
    for(std::size_t i = 1; i <events_.size(); ++i){
        if(events_[i].tick < events_[i-1].tick){    // 同一tickは許可するので`<`
            throw std::invalid_argument(
                "EnemySpawnEvent must be sorted by tick. "
            );
        }
    }
}

/**
 * @brief スポーンイベントを処理する
 *
 * events_はtickの昇順で並んでいる前提
 * stage_tickがevent.tickに到達したイベントをまとめて処理する
 * 
 * プールが満杯でスポーンできなかった場合はspawn_failedを増やして次へ進む
 * →リプレイ性を優先するためリトライはしない
 *
 * @param stage_tick 現在のステージtick
 * @param enemies    EnemySystemの参照
 * @return EnemySpawnTickResult 今回処理したスポーン結果
 */
EnemySpawnTickResult EnemySpawner::UpdateTick(
    std::uint64_t stage_tick,
    EnemySystem& enemies
)
{
    EnemySpawnTickResult result{};

    while(next_event_index_ < events_.size()){
        const auto& event = events_[next_event_index_];
        // まだ時刻が来ていないイベントは後回し
        if(event.tick > stage_tick){
            break;
        }
        // 本来起きない想定: stage_tickが飛んだ場合など
        if(event.tick < stage_tick){
            ++next_event_index_;
            ++result.spawn_failed;
            continue;
        }
        // スポーン実行して結果チェック
        if(enemies.Spawn(event.params)){
            ++result.spawned;
        } else {
            ++result.spawn_failed;
        }
        ++next_event_index_;
    }

    return result;
}

/**
 * @brief インデックスのみリセット
 *
 * ゲームリトライ時などに使用
 * イベントを再設定する場合はEnemySpawnerを再構築する
 */
void EnemySpawner::Reset() noexcept{
    next_event_index_ = 0;
}

}   // namespace zimovka
