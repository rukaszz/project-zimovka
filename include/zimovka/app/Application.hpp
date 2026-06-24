#ifndef APP_APPLICATION_HPP_
#define APP_APPLICATION_HPP_

namespace zimovka{

class Application{
public:
    // main.cppで呼び出す
    int run(int argc, char* argv[]);

private:
    // メインゲームループ制御
    bool running_ = true;
    
    // イベントの処理
    void processEvents();
    // ゲームの更新
    void update(float dt);
    // 描画処理
    void render();
};

}   // namespace zimovka

#endif  // APP_APPLICATION_HPP_
