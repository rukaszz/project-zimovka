#ifndef ZIMOVKA_INPUT_INPUTSYSTEM_HPP_
#define ZIMOVKA_INPUT_INPUTSYSTEM_HPP_

#include <array>

#include <SDL2/SDL.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/input/InputState.hpp"

namespace zimovka{
    
/**
 * @brief SDLイベントをActionに変換し，入力状態を管理するクラス
 * 
 */
class InputSystem{
public:
    // 状態の参照
    bool IsHeld(zimovka::Action act)     const;
    bool IsPressed(zimovka::Action act)  const;
    bool IsReleased(zimovka::Action act) const;
};

}   // namespace zimovka


#endif  // ZIMOVKA_INPUT_INPUTSYSTEM_HPP_
