#ifndef ZIMOVKA_PLATFORM_SDLCONTEXT_HPP_
#define ZIMOVKA_PLATFORM_SDLCONTEXT_HPP_

namespace zimovka{

/**
 * @brief SDL関係のサブシステムを管理するクラス
 * 各サブシステムの初期化処理が順番に実施されるため，
 * 失敗した場合すでに初期化済みのシステムが残ってしまう
 * →structでネストしてコンストラクタとデストラクタを構築することで対応
 *
 * 初期化順：メンバ宣言の上から下
 * 破棄順  ：メンバ宣言の下から上(逆順)
 */
class SdlContext{
public:
    // SDL本体
    struct Core{
        Core();
        ~Core();
    };
    // SDL_image
    struct Img{
        Img();
        ~Img();
    };
    // SDL_ttf
    struct Ttf{
        Ttf();
        ~Ttf();
    };
    // SDL_mixer(コーデック初期化)
    struct MixerCore{
        MixerCore();
        ~MixerCore();
    };
    // SDL_mixer(デバイスオープン)
    struct MixerDevice{
        MixerDevice();
        ~MixerDevice();
    };

    // 宣言順に初期化，逆順に破棄される
    Core        core;        // 1番目に初期化，5番目に破棄
    Img         img;         // 2番目に初期化，4番目に破棄
    Ttf         ttf;         // 3番目に初期化，3番目に破棄
    MixerCore   mixerCore;   // 4番目に初期化，2番目に破棄
    MixerDevice mixerDevice; // 5番目に初期化，1番目に破棄

    SdlContext() = default;
    ~SdlContext() = default;
    // コピー禁止(SDLリソースは一意)
    SdlContext(const SdlContext&) = delete;
    SdlContext& operator=(const SdlContext&) = delete;
};

}   // namespace zimovka

#endif  // ZIMOVKA_PLATFORM_SDLCONTEXT_HPP_
