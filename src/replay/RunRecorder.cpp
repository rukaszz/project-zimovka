#include "zimovka/replay/RunRecorder.hpp"

#include <algorithm>

namespace zimovka{

/**
 * @brief 入力記録を開始するための初期化を行う関数
 *
 * @param seed:           乱数のシード値
 * @param reserve_frames: framesのキャパシティ予約数(デフォルト: MAX_RECORD_FRAMES)
 */
void RunRecorder::Start(std::uint32_t seed, std::size_t reserve_frames){
    // 既存の記録をリセットしてから開始する
    Clear();

    record_.format_version = RUN_RECORD_FORMAT_VERSION;
    record_.simulation_hz = zimovka::SimulationConfig::SIMULATION_HZ;
    record_.random_seed = seed;

    record_.random_seed = seed;
    // キャパシティはMAX_RECORD_FRAMEを上限とする
    const std::size_t cap = std::min(
        reserve_frames,
        static_cast<std::size_t>(MAX_RECORD_FRAMES)
    );
    record_.frames.reserve(cap);
    recording_ = true;
}

/**
 * @brief 1フレーム分の入力を記録する関数
 *
 * Pause/QuitのビットはRECORD_ACTION_MASKによって除外される
 * MAX_RECORD_FRAMESに達した場合はrecord_.frame_limit_reached = trueを立てて自動停止する
 *
 * @param input
 * @return true:  記録成功（上限到達による自動停止フレームも含む）
 * @return false: 記録中でないため無視
 */
bool RunRecorder::Record(const InputState& input){
    // 記録停止中は即return
    if(!recording_){
        return false;
    }
    // 現在の入力値を取得(Pauseなどのゲームプレイに関係ないUI操作系は記録しない)
    record_.frames.push_back({
        input.GetHeldBits()     & RECORD_ACTION_MASK,
        input.GetPressedBits()  & RECORD_ACTION_MASK,
        input.GetReleasedBits() & RECORD_ACTION_MASK,
    });
    // フレーム上限到達で自動停止(frame_limit_reachedで途中終了を明示する)
    if(record_.frames.size() >= static_cast<std::size_t>(MAX_RECORD_FRAMES)){
        record_.frame_limit_reached = true;
        Stop();
    }
    return true;
}

/**
 * @brief プレイの記録を停止する
 *
 */
void RunRecorder::Stop() noexcept{
    recording_ = false;
}

/**
 * @brief プレイ記録を初期化する
 *
 */
void RunRecorder::Clear() noexcept{
    recording_ = false;
    record_.format_version = RUN_RECORD_FORMAT_VERSION; // RunRecordで定義
    record_.simulation_hz = zimovka::SimulationConfig::SIMULATION_HZ;
    record_.random_seed = 0;
    record_.frame_limit_reached = false;
    // capacityは維持して空にする
    record_.frames.clear();
}

/**
 * @brief 記録中かどうかを返す
 *
 * @return true  記録中
 * @return false 停止中
 */
bool RunRecorder::IsRecording() const noexcept{
    return recording_;
}

/**
 * @brief 記録を返す
 *
 * @return const RunRecord&
 */
const RunRecord& RunRecorder::GetRecord() const noexcept{
    return record_;
}

}   // namespace zimovka
