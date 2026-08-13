#ifndef ZIMOVKA_REPLAY_REPLAYINPUT_CODEC_HPP_
#define ZIMOVKA_REPLAY_REPLAYINPUT_CODEC_HPP_

#include <cstdint>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/replay/RunRecord.hpp"

/**
 * @brief InputStateのencode/decodeを行う
 * 
 * RunRecorderによるInputStateの記録
 * ・InputState→RecordedInputFrame
 * RunPlaybackによるInputStateの復元
 * ・RecordedInputFrame→InputState
 * 
 * これらの記録/復元の処理が分散しないように一箇所へ集約するためのヘルパ関数群
 */
namespace zimovka{

// 記録するビットの定義
// Pause/Quitなどリプレイに関係ない操作は記録しない
inline constexpr std::uint32_t RECORD_ACTION_MASK =
        ActionBit(Action::MoveUp)
      | ActionBit(Action::MoveDown)
      | ActionBit(Action::MoveLeft)
      | ActionBit(Action::MoveRight)
      | ActionBit(Action::Slow)
      | ActionBit(Action::Shoot)
      | ActionBit(Action::Bomb);

/**
 * @brief InputState→RecordedInputFrameへのエンコード
 * 
 * 入力された状態(InputState)に対して，記録するキーのみ取得するようにマスク
 * 
 * @param input 
 * @return RecordedInputFrame 
 */
[[nodiscard]]   // 戻り値の無視を禁止する
inline RecordedInputFrame EncodeRecordedInput(const InputState& input) noexcept{
    return RecordedInputFrame{
        input.GetHeldBits()     & RECORD_ACTION_MASK, 
        input.GetPressedBits()  & RECORD_ACTION_MASK, 
        input.GetReleasedBits() & RECORD_ACTION_MASK
    };
}

/**
 * @brief RecordedInputState→inputStateへのデコード
 * 
 * 記録された入力(RecordedInputFrame)に対して，InputStateに変換する
 * リプレイ用の記録が壊れたり，不要な上位bitが混入しないようにマスクはする
 * 
 * @param frame 
 * @return InputState 
 */
[[nodiscard]]   // 戻り値の無視を禁止する
inline InputState DecodeRecordedInput(const RecordedInputFrame& frame) noexcept{
    return InputState::FromBits(
        frame.held_bits     & RECORD_ACTION_MASK, 
        frame.pressed_bits  & RECORD_ACTION_MASK, 
        frame.released_bits & RECORD_ACTION_MASK
    );
}
}   // namespace zimovka

#endif  // ZIMOVKA_REPLAY_REPLAYINPUT_CODEC_HPP_
