#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>

#include <SDL2/SDL.h>

#include "zimovka/rendering/texture/Texture.hpp"

using zimovka::Texture;

// ──────────────────────────────────────────────────────
// デフォルト構築
// ──────────────────────────────────────────────────────
/**
 * @brief デフォルトコンストラクタでインスタンス化
 * 
 */
TEST(TextureTest, DefaultConstructed_IsInvalid){
    Texture t;
    EXPECT_FALSE(t.IsValid());  // texture_はnullptr
}
/**
 * @brief デフォルトコンストラクタではメンバ変数も初期値である
 * 
 */
TEST(TextureTest, DefaultConstructed_WidthAndHeightAreZero){
    Texture t;
    EXPECT_EQ(t.Width(), 0);
    EXPECT_EQ(t.Height(), 0);
}
/**
 * @brief デフォルトコンストラクタによる構築後のGet()呼び出し
 * 
 */
TEST(TextureTest, DefaultConstructed_GetReturnsNull){
    Texture t;
    EXPECT_EQ(t.Get(), nullptr);    // texture_はnullptr
}

// ──────────────────────────────────────────────────────
// Load() の引数チェック
// ──────────────────────────────────────────────────────
// 
/**
 * @brief RendererがnullptrだったときのTHROWチェック
 * 
 * renderer=nullptr→std::invalid_argument(SDLの初期化不要で確認できる境界チェック)
 */
TEST(TextureTest, Load_NullRenderer_ThrowsInvalidArgument){
    Texture t;
    EXPECT_THROW(
        t.Load(nullptr, "any_path.png"),
        std::invalid_argument
    );
    // 例外後もTextureは無効のまま
    EXPECT_FALSE(t.IsValid());
}

// ──────────────────────────────────────────────────────
// Reset()
// ──────────────────────────────────────────────────────
/**
 * @brief デフォルトコンストラクタでのインスタンス後にReset()を呼び出してもクラッシュしないか確認
 * 
 */
TEST(TextureTest, Reset_OnDefaultConstructed_DoesNotCrash){
    Texture t;
    EXPECT_NO_THROW(t.Reset());
    EXPECT_FALSE(t.IsValid());
}

// ──────────────────────────────────────────────────────
// ムーブセマンティクス(デフォルト構築テクスチャ同士)
// ──────────────────────────────────────────────────────
/**
 * @brief dstをムーブで構成(src→dst)
 * 
 */
TEST(TextureTest, MoveConstruct_SourceBecomesInvalid){
    Texture src;
    Texture dst(std::move(src));
    // どちらも元々invalid = ムーブ後もinvalid
    EXPECT_FALSE(dst.IsValid());
    // ムーブ後の呼び出しでエラー
    EXPECT_FALSE(src.IsValid());    // NOLINT(bugprone-use-after-move)
}

/**
 * @brief dstへsrcをムーブ
 * 
 */
TEST(TextureTest, MoveAssign_SourceBecomesInvalid){
    Texture src;
    Texture dst;
    dst = std::move(src);
    EXPECT_FALSE(dst.IsValid());
    EXPECT_FALSE(src.IsValid());    // NOLINT(bugprone-use-after-move)
}

/**
 * @brief 自己代入によるムーブ
 * 
 */
TEST(TextureTest, MoveAssign_SelfAssign_DoesNotCrash){
    Texture t;
    // 自己代入(同一インスタンスかのチェックがあるのでエラーにならない)
    Texture& ref = t;
    EXPECT_NO_THROW(ref = std::move(t));
}

// ──────────────────────────────────────────────────────
// パス文字列変換(Windows/Linux移植性の契約テスト)
//
// Windowsではstd::filesystem::path::string()がANSIエンコーディングを返す.
// 非ASCII文字を含む場合, u8string()またはgeneric_string()を使う必要がある.
// 以下のテストは「サブディレクトリ付きパスが意図した文字列に変換されること」を
// generic_string()で確認し, パス連結の正確性を保証する.
// ──────────────────────────────────────────────────────
/**
 * @brief rootパスとファイルパスの組み合わせ文字列の検証
 * 
 */
TEST(TexturePathTest, PathConcatenation_ForwardSlash){
    const std::filesystem::path root = "assets_stab";
    const std::filesystem::path rel  = "img/player/player.png";
    // generic_string() はプラットフォーム問わず '/' 区切りを返す
    EXPECT_EQ((root / rel).generic_string(), "assets_stab/img/player/player.png");
}
/**
 * @brief 複数のセグメントで分割したパスを構成し検証
 * 
 */
TEST(TexturePathTest, PathConcatenation_MultipleSegments){
    const std::filesystem::path p =
        std::filesystem::path("assets") / "img" / "bullet" / "player_bullet.png";
    EXPECT_EQ(p.generic_string(), "assets/img/bullet/player_bullet.png");
}

// ──────────────────────────────────────────────────────
// 実画像ファイルを使ったロードテスト
//
// SetUpTestSuiteでSDLを一度だけ初期化し，4×4 BMPをZIMOVKA_TEST_IMAGE_DIRに生成する
// TearDownTestSuiteで画像ディレクトリごと削除しSDLを終了することで，テスト完了前後の状態は一致する
// ──────────────────────────────────────────────────────
namespace{
/**
 * @brief テスト用フィクスチャ
 * 
 * SDL関係とファイルシステムを用いてTextureに必要な処理を実装
 */
class TextureLoadTest : public ::testing::Test{
protected:
    static SDL_Window*           window_;
    static SDL_Renderer*         renderer_;
    static std::filesystem::path valid_image_path_;
    static bool                  sdl_ready_;    // SDLの初期化処理がうまく行ったか

    /**
     * @brief SDLの設定とSDL初期化，画像の作成などを行うセットアップ関数
     * 
     */
    static void SetUpTestSuite(){
        // 画面描画は不要なのでオフスクリーンドライバを使用
        SDL_setenv("SDL_VIDEODRIVER", "offscreen", /*overwrite=*/1);
        if(SDL_Init(SDL_INIT_VIDEO) != 0){
            return;
        }
        // window, rendererの初期化
        window_ = SDL_CreateWindow("test", 0, 0, 4, 4, SDL_WINDOW_HIDDEN);
        if(!window_){
            SDL_Quit(); 
            return; 
        }

        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        if(!renderer_){
            SDL_DestroyWindow(window_);
            window_ = nullptr;
            SDL_Quit();
            return;
        }

        // テスト用4×4BMPを生成
        namespace fs = std::filesystem;
        // ディレクトリはCMakeListsで定義
        const fs::path dir = fs::path(ZIMOVKA_TEST_IMAGE_DIR);
        // ディレクトリ，画像を生成
        fs::create_directories(dir);
        valid_image_path_ = dir / "test_4x4.bmp";
        // サーフェスから直接BMPで保存
        // flags(常に0)，サーフェス幅，高さ，ビット深度，...
        SDL_Surface* surf = SDL_CreateRGBSurface(0, 4, 4, 24, 0, 0, 0, 0);
        if(surf){
            SDL_SaveBMP(surf, valid_image_path_.string().c_str());
            SDL_FreeSurface(surf);
        }

        sdl_ready_ = (renderer_ != nullptr) && fs::exists(valid_image_path_);
    }

    /**
     * @brief テスト用の処理の片付けを行う関数
     * 
     */
    static void TearDownTestSuite(){
        namespace fs = std::filesystem;
        const fs::path dir = fs::path(ZIMOVKA_TEST_IMAGE_DIR);
        if(fs::exists(dir)){
            fs::remove_all(dir);    // 作成したディレクトリまるごと削除
        }
        // リソース解放
        if(renderer_){
            SDL_DestroyRenderer(renderer_);
            renderer_ = nullptr;
        }
        if(window_){
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        SDL_Quit();
    }

    void SetUp() override{
        if(!sdl_ready_){
            GTEST_SKIP() << "SDL renderer not available";
        }
    }
};

SDL_Window*           TextureLoadTest::window_           = nullptr;
SDL_Renderer*         TextureLoadTest::renderer_         = nullptr;
std::filesystem::path TextureLoadTest::valid_image_path_;
bool                  TextureLoadTest::sdl_ready_        = false;

} // namespace

/**
 * @brief 有効な画像を読み込むと，そのテクスチャが有効になる
 * 
 */
TEST_F(TextureLoadTest, Load_ValidImage_BecomesValid){
    Texture t;
    t.Load(renderer_, valid_image_path_);
    EXPECT_TRUE(t.IsValid());
}

/**
 * @brief 有効な画像を読み込むと，その寸法が保存される
 * 
 */
TEST_F(TextureLoadTest, Load_ValidImage_StoresDimensions){
    Texture t;
    t.Load(renderer_, valid_image_path_);
    // 4×4BMP
    EXPECT_EQ(t.Width(),  4);
    EXPECT_EQ(t.Height(), 4);
}

/**
 * @brief 読み込まれたテクスチャをリセットすると，そのテクスチャは無効になる
 * 
 */
TEST_F(TextureLoadTest, Reset_LoadedTexture_BecomesInvalid){
    Texture t;
    t.Load(renderer_, valid_image_path_);
    ASSERT_TRUE(t.IsValid());
    // 読み込んだ後にリセット
    t.Reset();
    EXPECT_FALSE(t.IsValid());
    EXPECT_EQ(t.Width(),  0);
    EXPECT_EQ(t.Height(), 0);
}

/**
 * @brief Move Constructを実行すると，読み込まれたテクスチャの所有権が移転される
 *
 * srcが有効な状態でムーブ構築すると：
 * - dstは有効
 * - srcは無効※所有権がdstに移転
 */
TEST_F(TextureLoadTest, MoveConstruct_LoadedTexture_TransfersOwnership){
    Texture src;
    src.Load(renderer_, valid_image_path_);
    ASSERT_TRUE(src.IsValid());
    // srcは所有権を放棄
    Texture dst(std::move(src));

    EXPECT_TRUE(dst.IsValid());
    EXPECT_FALSE(src.IsValid());    // NOLINT(bugprone-use-after-move)
}

/**
 * @brief 再読み込みに失敗しても，以前のテクスチャは保持される
 *
 * Load()は新テクスチャ生成に成功してから旧テクスチャを破棄するため，
 * 無効パスへのLoad()によって失敗した場合，元テクスチャは残存する
 */
TEST_F(TextureLoadTest, ReloadFailure_PreservesPreviousTexture){
    // prev-image load
    Texture t;
    t.Load(renderer_, valid_image_path_);
    ASSERT_TRUE(t.IsValid());
    // invalid-image load
    const std::filesystem::path invalid_path = ZIMOVKA_TEST_IMAGE_DIR "/nonexistent.png";
    EXPECT_THROW(t.Load(renderer_, invalid_path), std::runtime_error);

    // 失敗後も元テクスチャが保持されている
    EXPECT_TRUE(t.IsValid());
    EXPECT_EQ(t.Width(),  4);
    EXPECT_EQ(t.Height(), 4);
}
