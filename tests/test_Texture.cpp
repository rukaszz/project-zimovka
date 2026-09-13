#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>

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
/**
 * @brief string()とgeneric_string()の一致を検証
 * 
 * ※string() のエンコーディングに関する注意:
 *     Linux: UTF-8 → Windows: ASCII-only ←このパスはstring()==generic_string()で一致
 */
TEST(TexturePathTest, PathString_AsciiOnly_MatchesGeneric){
    const std::filesystem::path p = "assets_stab/img/player/player.png";
    // ASCII-onlyパスではstring()とgeneric_string()が一致する(Linux)
    EXPECT_EQ(p.string(), p.generic_string());
}
