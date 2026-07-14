#ifndef ZIMOVKA_REPLAY_RUNRECORDERSYSTEM_HPP_
#define ZIMOVKA_REPLAY_RUNRECORDERSYSTEM_HPP_

#include <cstdint>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/replay/RunRecorder.hpp"

namespace zimovka{
/**
 * @brief リプレイ用に実行時のプレイヤーの入力を管理するクラス
 *
 */
class RunRecorderSystem{
private:
    // 20 * 60s * 60fps = 72,000 frameを確保しておく
    static constexpr int MAX_RECORD_FRAME = 72000;
    // Pause/Quit はゲームを止める操作なのでリプレイに記録しない
    static constexpr std::uint32_t RECORD_MASK =
        ~(ActionBit(Action::Pause) | ActionBit(Action::Quit));
    // 記録する/しないの管理
    bool recording_ = false;
    // 入力記録用構造体
    RunRecord record_;

public:
    // 記録開始
    void Start(std::uint32_t seed, std::uint32_t reserve_frames = MAX_RECORD_FRAME);
    // 記録
    void Record(const InputState& input);
    // 記録停止
    void Stop() noexcept;
    // 初期化
    void Clear() noexcept;
    // getter
    bool IsRecording() const noexcept;
    const RunRecord& GetRecord() const noexcept;
};
}   // namespace zimovka

#endif  // ZIMOVKA_REPLAY_RUNRECORDERSYSTEM_HPP_
