#include <gtest/gtest.h>

#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "zimovka/rendering/overlay/GameTextOverlay.hpp"
#include "zimovka/rendering/overlay/GameTextOverlayViewModel.hpp"

using zimovka::GameTextOverlay;
using zimovka::GameTextOverlayViewModel;

// ──────────────────────────────────────────────────────
// SDL不要テスト (RenderPipelineへのViewModel渡しを含む基本動作)
// ──────────────────────────────────────────────────────
/**
 * @brief デフォルトコンストラクタだけでは未初期化状態であるか
 * 
 */
TEST(GameTextOverlayBasicTest, DefaultConstructed_IsNotLoaded){
    GameTextOverlay overlay;
    EXPECT_FALSE(overlay.IsLoaded());
}

/**
 * @brief デフォルトコンストラクタ後のReset()はクラッシュしない
 * 
 * ※TextTextureとメンバ変数の解放のみなので問題ない
 */
TEST(GameTextOverlayBasicTest, Reset_OnDefaultConstructed_DoesNotCrash){
    GameTextOverlay overlay;
    EXPECT_NO_THROW(overlay.Reset());
    EXPECT_FALSE(overlay.IsLoaded());
}

/**
 * @brief renderer=nullptrでLoad()するとstd::invalid_argumentが投げられる
 * 
 */
TEST(GameTextOverlayBasicTest, Load_NullRenderer_Throws){
    GameTextOverlay overlay;
    EXPECT_THROW(
        overlay.Load(nullptr, "any_font.ttf", 24),
        std::invalid_argument
    );
    EXPECT_FALSE(overlay.IsLoaded());
}

/**
 * @brief ステージ進行中(show_stage_clear=false)はRendererなしでもRender()がtrueを返す
 *
 * RenderPipeline内ではStageStatement::Cleared以外のときshow_stage_clear=falseがViewへ渡る
 * GameTextOverlay::Render()は先頭のガード節で即returnでtrueを返す
 * →Rendererが未初期化でもSDLを呼び出さずtrueを返す
 * ※つまりSDLが不要なテスト
 */
TEST(GameTextOverlayBasicTest, Overlay_HiddenBeforeStageClear){
    GameTextOverlay overlay;    // Load()なし
    GameTextOverlayViewModel model{};
    model.show_stage_clear = false;
    EXPECT_TRUE(overlay.Render(model));
    // Load()後もshow_stage_clear=falseなら描画しない(変化なし)
    EXPECT_FALSE(overlay.IsLoaded());
}

// ──────────────────────────────────────────────────────
// SDL + SDL_ttf が必要なテスト
//
// SetUpTestSuiteでオフスクリーンSDLを一度だけ初期化し，
// TearDownTestSuiteで破棄する
// フォントパスはCMakeのコンパイル定義ZIMOVKA_TEST_FONT_PATHで渡す
// ──────────────────────────────────────────────────────
namespace{

class GameTextOverlayLoadTest : public ::testing::Test{
protected:
    static SDL_Window*   window_;
    static SDL_Renderer* renderer_;
    static bool          sdl_ready_;
    // テスト用SDLの初期化
    static void SetUpTestSuite(){
        // 画面描画不要なのでオフスクリーンドライバを使用
        SDL_setenv("SDL_VIDEODRIVER", "offscreen", /*overwrite=*/1);
        if(SDL_Init(SDL_INIT_VIDEO) != 0){
            return; 
        }
        if(TTF_Init() != 0){
            SDL_Quit();
            return;
        }
        window_ = SDL_CreateWindow("test", 0, 0, 1, 1, SDL_WINDOW_HIDDEN);
        if(!window_){
            TTF_Quit();
            SDL_Quit();
            return;
        }
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        if(!renderer_){
            SDL_DestroyWindow(window_);
            window_ = nullptr;
            TTF_Quit();
            SDL_Quit();
            return;
        }
        sdl_ready_ = true;
    }
    // 各種リソースの解放※RAIIに従う
    static void TearDownTestSuite(){
        if(renderer_){
            SDL_DestroyRenderer(renderer_); 
            renderer_ = nullptr;
        }
        if(window_){
            SDL_DestroyWindow(window_);  
            window_ = nullptr; 
        }
        TTF_Quit();
        SDL_Quit();
        sdl_ready_ = false;
    }
    // SDLの準備ができなければテストスキップ
    void SetUp() override{
        if(!sdl_ready_){
            GTEST_SKIP() << "SDL+TTF renderer not available";
        }
    }
};

SDL_Window*   GameTextOverlayLoadTest::window_    = nullptr;
SDL_Renderer* GameTextOverlayLoadTest::renderer_  = nullptr;
bool          GameTextOverlayLoadTest::sdl_ready_ = false;

} // anonymous namespace

/**
 * @brief 有効なフォントパスでのLoad()はIsLoaded()がtrueになる
 * 
 */
TEST_F(GameTextOverlayLoadTest, Load_ValidFont_IsLoaded){
    GameTextOverlay overlay;
    EXPECT_NO_THROW(overlay.Load(renderer_, ZIMOVKA_TEST_FONT_PATH, 24));
    EXPECT_TRUE(overlay.IsLoaded());
}

/**
 * @brief 存在しないフォントパスでLoad()するとstd::runtime_errorが投げられる
 * 
 */
TEST_F(GameTextOverlayLoadTest, Load_InvalidFontPath_Throws){
    GameTextOverlay overlay;
    EXPECT_THROW(
        overlay.Load(renderer_, "/nonexistent/path/font.ttf", 24),
        std::runtime_error
    );
    // TTF_OpenFont失敗しておりIsLoaded()はfalseのまま
    EXPECT_FALSE(overlay.IsLoaded());
}

/**
 * @brief font_size=0でLoad()するとstd::invalid_argumentが投げられる
 * 
 * ※フォントサイズのバリデーションはTTF_OpenFont呼び出しの前に行われる
 */
TEST_F(GameTextOverlayLoadTest, Load_InvalidFontSize_Throws){
    GameTextOverlay overlay;
    EXPECT_THROW(
        overlay.Load(renderer_, ZIMOVKA_TEST_FONT_PATH, 0),
        std::invalid_argument
    );
    EXPECT_FALSE(overlay.IsLoaded());
}

/**
 * @brief Load()後にReset()するとIsLoaded()がfalseになる
 * 
 */
TEST_F(GameTextOverlayLoadTest, Reset_AfterLoad_IsNotLoaded){
    GameTextOverlay overlay;
    // Load()は成功
    overlay.Load(renderer_, ZIMOVKA_TEST_FONT_PATH, 24);
    ASSERT_TRUE(overlay.IsLoaded());
    overlay.Reset();
    EXPECT_FALSE(overlay.IsLoaded());
}

/**
 * @brief Load()を2回連続で呼び出してもクラッシュせず，IsLoaded()がtrueのままである
 *
 * 2回目のLoad()内でReset()が呼ばれ，1回目に確保したTTF_Fontが正しく終了すること，
 * ASANやLSAN等でメモリリークが生じないことを合わせて確認
 */
TEST_F(GameTextOverlayLoadTest, Overlay_ReinitializeResources){
    GameTextOverlay overlay;
    // Load()1回目
    EXPECT_NO_THROW(overlay.Load(renderer_, ZIMOVKA_TEST_FONT_PATH, 24));
    ASSERT_TRUE(overlay.IsLoaded());
    // 2回目: 1回目のTTF_FontはReset()で解放されてから新しいフォントが設定される
    EXPECT_NO_THROW(overlay.Load(renderer_, ZIMOVKA_TEST_FONT_PATH, 36));
    EXPECT_TRUE(overlay.IsLoaded());
}
