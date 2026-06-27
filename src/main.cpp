#include "zimovka/app/Application.hpp"

/**
 * @brief Zimovka起動時のエントリポイント
 * 最小限の処理のみ記述する
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char* argv[]){
    // ゲーム起動
    zimovka::Application app;
    return app.Run(argc, argv);
}
