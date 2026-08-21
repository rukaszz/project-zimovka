#include "zimovka/replay/RunPlayback.hpp"

#include "zimovka/replay/ReplayInputCodec.hpp"

namespace zimovka{
/**
 * @brief リプレイ再生スタート
 * 
 * @param record 
 * @return PlaybackStartResult 
 */
PlaybackStartResult RunPlayback::Start(const RunRecord& record) noexcept{
    // 最初に停止処理
    Stop();
    // リプレイの状態チェック処理
    if(record.format_version != RUN_RECORD_FORMAT_VERSION){
        return PlaybackStartResult::UnsupportedFormat;
    }
    if(record.simulation_hz != SimulationConfig::SIMULATION_HZ){
        return PlaybackStartResult::SimulationHzMismatch;
    }
    if(record.rng_version != GAMEPLAY_RNG_VERSION){
        return PlaybackStartResult::RngVersionMismatch;
    }
    if(record.frame_limit_reached){
        return PlaybackStartResult::IncompleteRecord;
    }
    if(record.frames.empty()){
        return PlaybackStartResult::EmptyRecord;
    }
    // チェック処理完了後，渡されたレコードを参照する
    record_ = &record;
    next_frame_ = 0;

    return PlaybackStartResult::Started;
}

/**
 * @brief 再生の停止
 * 
 */
void RunPlayback::Stop() noexcept{
    record_ = nullptr;
    next_frame_ = 0;
}

/**
 * @brief リプレイ記録の次フレームを取得する
 * 
 * @return std::optional<InputState> 
 */
std::optional<InputState> RunPlayback::ConsumeNextInput() noexcept{
    // 記録が無効 または 最後のフレームに達している
    if(!record_ || next_frame_ >= record_->frames.size()){
        return std::nullopt;    // optionalのNULL(未初期化状態)
    }
    // 次フレーム取得
    const RecordedInputFrame& frame = record_->frames[next_frame_];
    // indexを進める
    ++next_frame_;
    // デコードしたフレームを返す
    return DecodeRecordedInput(frame);
}

}   // namespace zimovka
