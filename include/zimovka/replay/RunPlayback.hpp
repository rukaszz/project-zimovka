#ifndef ZIMOVKA_REPLAY_RUNPLAYBACK_HPP_
#define ZIMOVKA_REPLAY_RUNPLAYBACK_HPP_

#include <cstddef>
#include <optional>

#include "zimovka/input/InputState.hpp"
#include "zimovka/replay/RunRecord.hpp"

/**
 * @brief リプレイを再生する責務を担う
 * 
 */
namespace zimovka{
    
/**
 * @brief 再生の状態管理用
 * 
 * 状態として保持しないので，finishedは不要
 * frame.size()で最後まで達したか見る
 */
enum class PlaybackStartResult{
    Started,                // 正常に開始
    UnsupprtedFormat,       // フォーマットの不一致
    SimulationHzMismatch,   // 再現用FPS設定値の不一致
    RngVersionMismatch,     // 乱数バージョン不一致
    IncompleteRecord        // 記録不完全
};

/**
 * @brief リプレイ再生管理用クラス
 * 
 */
class RunPlayback{
private:
    // 記録
    // Null(失敗)の可能性を考慮してポインタで保持
    // RunRecordを所有せず変更しない
    const RunRecord* record_ = nullptr;
    // 次の記録フレームを指す
    std::size_t next_frame_ = 0;

public:
    // リプレイ開始
    PlaybackStartResult Start(const RunRecord& record) noexcept;
    // 停止
    void Stop() noexcept;
    // 次のフレームへ進む
    [[nodiscard]]
    std::optional<InputState> ConsumeNextInput() noexcept;

    // 状態getter
    bool IsPlaying() const noexcept{
        return record_ != nullptr;
    }
    bool IsFinished() const noexcept{
        // 記録できており かつ 記録の最後まで達した
        return record_ != nullptr 
            && next_frame_ >= record_->frames.size();
    }
    std::size_t GetNextFrameIndex() const noexcept{
        return next_frame_;
    }
};

} // namespace zimovka


#endif  // ZIMOVKA_REPLAY_RUNPLAYBACK_HPP_
