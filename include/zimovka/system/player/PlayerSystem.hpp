#ifndef ZIMOVKA_SYSTEM_PLAYER_PLAYERSYSTEM_HPP_
#define ZIMOVKA_SYSTEM_PLAYER_PLAYERSYSTEM_HPP_

#include "zimovka/input/InputState.hpp"
#include "zimovka/system/player/Player.hpp"

namespace zimovka{

class PrimitiveRenderer;

class PlayerSystem{
private:
    // プレイヤー構造体
    Player player_;
    // 画面の大きさ
    float screen_width_ = 960.0f;
    float screen_height_ = 720.0f;

    // 画面外に出ないようにclampする関数
    void ClampToScreen();

public:
    // 初期化
    void Initialize(float screen_width, float screen_height);
    // 更新
    void Update(float dt, const InputState& input);
    // 描画
    void Render(PrimitiveRenderer& renderer) const;
    // getter
    const Player& GetPlayer() const noexcept{
        return player_;
    }
};

}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEM_PLAYER_PLAYERSYSTEM_HPP_
