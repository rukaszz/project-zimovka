#include "zimovka/app/Application.hpp"

#include <iostream>

#include "zimovka/platform/SdlContext.hpp"
#include "zimovka/platform/Window.hpp"
#include "zimovka/rendering/Renderer.hpp"

/**
 * @brief ゲーム実行関数
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int zimovka::Application::run(int argc, char* argv[]){
    // 警告無効化のためにコマンドライン引数無効化
    (void)argc;
    (void)argv;
    // SDLの初期化
    SdlContext sdl;
    // Window/Rendererの準備
    Window window("Zimovka", 960, 720);
    Renderer renderer(window.get());
    // ループ処理用
    running_ = true;
    // ゲームループ
    while(running_){
        // 最初にイベント処理
        processEvents();
        // ゲーム更新
        // update(1.0f / 60.0f);
        // 消去→配置→描画
        renderer.clear();
        // render();
        renderer.setDrawColoer();
        renderer.present();
    }

    return 0;
}

/**
 * @brief イベント処理用関数
 * 
 * 現状は簡単なキー入力を処理する
 */
void zimovka::Application::processEvents(){
    // イベント取得
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        // 終了要求を受けたら終了
        if(event.type == SDL_QUIT){
            running_ = false;
        }
        // ESCキー押下で終了
        if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
            running_ = false;
        }
    }
}

void zimovka::Application::render(){
    
}
