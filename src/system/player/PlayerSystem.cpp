#include "zimovka/system/player/PlayerSystem.hpp"

#include <cmath>

#include <SDL2/SDL.h>

#include "zimovka/input/Action.hpp"
#include "zimovka/rendering/PrimitiveRenderer.hpp"

namespace zimovka{

/**
 * @brief Player, PlayerSystemの初期化
 * 
 * @param screen_width 
 * @param screen_height 
 */
void zimovka::PlayerSystem::Initialize(float screen_width, float screen_height){
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
void zimovka::PlayerSystem::Update(float dt, const InputState& input){
    // 移動方向ベクトル
    float dx = 0.0f;
    float dy = 0.0f;
    Vec2 dv(0.0f, 0.0f);
    // 入力処理(移動)
    if(input.IsHeld(zimovka::Action::MoveLeft)){
        dx -= 1.0f;
        dv.x -= 1.0f;
    }
    if(input.IsHeld(zimovka::Action::MoveRight)){
        dx += 1.0f;
        dv.x += 1.0f;
    }
    if(input.IsHeld(zimovka::Action::MoveUp)){
        dy -= 1.0f;
        dv.y -= 1.0f;
    }
    if(input.IsHeld(zimovka::Action::MoveDown)){
        dy += 1.0f;
        dv.y += 1.0f;
    }
    // 斜め移動
    if(!dv.IsZero()){
        dv = dv.Normalized();
    }
    // 移動速度決定
    const float speed = input.IsHeld(zimovka::Action::Slow)
        ? player_.slow_speed : player_.speed;
    const float move_speed = speed * dt;
    // 座標計算
    player_.position.x += dv.x * move_speed;
    player_.position.y += dv.y * move_speed;
    // 画面外に出るなら補正
    ClampToScreen();
}

/**
 * @brief プレイヤーの描画処理
 * 現状は矩形と円で簡易的に表示する
 * 
 * @param renderer 
 */
void zimovka::PlayerSystem::Render(PrimitiveRenderer& renderer) const{
    // 色の定義
    const SDL_Color body_color{80, 180, 255, 255};
    const SDL_Color hitbox_color{255, 255, 255, 255};
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
void zimovka::PlayerSystem::ClampToScreen(){
    // プレイヤーの幅/高さの半分を取得
    const float half_w = player_.width * 0.5f;
    const float half_h = player_.height * 0.5f;
    // プレイヤーの身体が半分以上画面左端からでるなら，半分だけめり込むように補正
    if(player_.position.x < half_w){
        player_.position.x = half_w;
    }
    // プレイヤーの身体が半分以上画面右端からでるなら，半分までめり込むように補正
    if(player_.position.x > screen_width_ - half_w){
        player_.position.x = screen_width_ - half_w;
    }
    // プレイヤーの身体が半分以上画面上端からでるなら，半分までめり込むように補正
    if(player_.position.y < half_h){
        player_.position.y = half_h;
    }
    // プレイヤーの身体が半分以上画面下端からでるなら，半分までめり込むように補正
    if(player_.position.y > screen_height_ - half_h){
        player_.position.y = screen_height_ - half_h;
    }
}

}
