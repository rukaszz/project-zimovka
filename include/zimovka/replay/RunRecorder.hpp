#ifndef ZIMOVKA_REPLAY_RUNRECORDER_HPP_
#define ZIMOVKA_REPLAY_RUNRECORDER_HPP_

#include <cstddef>
#include <cstdint>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/replay/RunRecord.hpp"

namespace zimovka{
/**
 * @brief リプレイ用に実行時のプレイヤーの入力を管理するクラス
 *
 */
class RunRecorder{
public:
    // 20 * 60s * 60fps = 72,000 frameを確保しておく
    static constexpr std::size_t MAX_RECORD_FRAMES = 72000; // 定数として外部へ晒す

private:
    // 記録する/しないの管理
    bool recording_ = false;
    // 入力記録用構造体
    RunRecord record_;

    // Pause/Quitなどリプレイに関係ない操作は記録しない
    // 記録するビットの定義
    static constexpr std::uint32_t RECORD_ACTION_MASK =
        ActionBit(Action::MoveUp)
      | ActionBit(Action::MoveDown)
      | ActionBit(Action::MoveLeft)
      | ActionBit(Action::MoveRight)
      | ActionBit(Action::Slow)
      | ActionBit(Action::Shoot)
      | ActionBit(Action::Bomb);
    
public:
    // 記録開始
    void Start(std::uint32_t seed, std::size_t reserve_frames = MAX_RECORD_FRAMES);
    // 記録
    bool Record(const InputState& input);
    // 記録停止
    void Stop() noexcept;
    // 初期化
    void Clear() noexcept;
    // getter
    bool IsRecording() const noexcept;
    const RunRecord& GetRecord() const noexcept;
};
}   // namespace zimovka

#endif  // ZIMOVKA_REPLAY_RUNRECORDER_HPP_
