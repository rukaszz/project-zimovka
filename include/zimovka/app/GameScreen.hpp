#ifndef ZIMOVKA_APP_GAMESCREEN_HPP_
#define ZIMOVKA_APP_GAMESCREEN_HPP_

namespace zimovka{
/**
 * @brief ゲームの状態を管理するためのenum
 * 
 * GameScreenはゲームの画面の状態，実行状態を表す
 */
enum class GameScreen{
    Title, 
    Playing, 
    Result, 
    GameOver, 
};
} // namespace zimovka

#endif  // ZIMOVKA_APP_GAMESCREEN_HPP_
