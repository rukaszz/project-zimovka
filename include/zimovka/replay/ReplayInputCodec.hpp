#ifndef ZIMOVKA_REPLAY_REPLAYINPUT_CODEC_HPP_
#define ZIMOVKA_REPLAY_REPLAYINPUT_CODEC_HPP_

#include <cstdint>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"
#include "zimovka/replay/RunRecord.hpp"

namespace zimovka{
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



}   // namespace zimovka

#endif  // ZIMOVKA_REPLAY_REPLAYINPUT_CODEC_HPP_
