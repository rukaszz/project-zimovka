#include "zimovka/systems/player/PlayerSystem.hpp"

#include <cmath>

#include "zimovka/rendering/Color.hpp"
#include "zimovka/input/Action.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"

namespace zimovka{

/**
 * @brief Player, PlayerSystemの初期化
 * 
 * @param screen_width 
 * @param screen_height 
 */
void PlayerSystem::Initialize(float screen_width, float screen_height){
    // メンバ変数初期化
    screen_width_ = screen_width;
    screen_height_ = screen_height;
    // プレイヤーは画面中心へ出す
    player_.position.x = screen_width_ * 0.5f;
    player_.position.y = screen_height_ * 0.8f; // 画面下方にだす
}

/**
 * @brief プレイヤーに関する更新処理
 * 現状はキー入力に応じた移動処理のみ
 * 
 * @param dt 
 * @param input 
 */
void PlayerSystem::Update(float dt, const InputState& input){
    // 移動方向ベクトル
    Vec2 dir(0.0f, 0.0f);
    // 入力処理(移動)
    if(input.IsHeld(zimovka::Action::MoveLeft)){
        dir.x -= 1.0f;
    }
    if(input.IsHeld(zimovka::Action::MoveRight)){
        dir.x += 1.0f;
    }
    if(input.IsHeld(zimovka::Action::MoveUp)){
        dir.y -= 1.0f;
    }
    if(input.IsHeld(zimovka::Action::MoveDown)){
        dir.y += 1.0f;
    }
    // 斜め移動
    if(!dir.IsZero()){
        dir = dir.Normalized();
    }
    // 移動速度決定
    const float speed = input.IsHeld(zimovka::Action::Slow)
        ? player_.slow_speed : player_.normal_speed;
    // 座標計算
    player_.position += dir * (speed * dt);
    // 画面外に出るなら補正
    ClampToScreen();
}

/**
 * @brief プレイヤーの描画処理
 * 現状は矩形と円で簡易的に表示する
 * 
 * @param renderer 
 */
void PlayerSystem::Render(PrimitiveRenderer& renderer) const{
    // 色の定義
    const Color body_color{80, 180, 255, 255};
    const Color hitbox_color{255, 255, 255, 255};
    // 左辺と上辺を定義
    const float left = player_.position.x - player_.width * 0.5f;
    const float top = player_.position.y - player_.height * 0.5f;
    // プレイヤー外見を矩形で表示
    renderer.DrawFilledRect(left, top, player_.width, player_.height, body_color);
    // プレイヤーの当たり判定は円で表現
    renderer.DrawFilledCircle(
        player_.position.x, 
        player_.position.y, 
        static_cast<int>(player_.hit_radius), 
        hitbox_color
    );
}

/**
 * @brief プレイヤー座標が画面外に出ないように補正する
 * 
 */
void PlayerSystem::ClampToScreen(){
    // プレイヤーの幅/高さから中心座標を取得
    const float half_w = player_.width * 0.5f;
    const float half_h = player_.height * 0.5f;
    // プレイヤーの身体が画面左端からでるなら補正
    if(player_.position.x < half_w){
        player_.position.x = half_w;
    }
    // プレイヤーの身体が画面右端からでるなら補正
    if(player_.position.x > screen_width_ - half_w){
        player_.position.x = screen_width_ - half_w;
    }
    // プレイヤーの身体が画面上端からでるなら補正
    if(player_.position.y < half_h){
        player_.position.y = half_h;
    }
    // プレイヤーの身体が画面下端からでるなら補正
    if(player_.position.y > screen_height_ - half_h){
        player_.position.y = screen_height_ - half_h;
    }
}

}   // namespace zimovka
