#ifndef PLATFORM_SDLCONTEXT_H_
#define PLATFORM_SDLCONTEXT_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <stdexcept>

/**
 * @brief SDL関係のサブシステムを管理するシステム
 * 各サブシステムの初期化処理が順番に実施されるため，
 * 失敗した場合すでに初期化済みのシステムが残ってしまう
 * →structでネストしてコンストラクタとデストラクタを構築することで対応
 * 
 */
class SdlSystem{
public:
    // 内部に個別のRAIIラッパを初期化の順番で宣言する
    // 初期化順：上から下，破棄順：下から上(逆順でcleanupしていく)
    // SDL初期化
    struct Core{
        Core(){
            if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0){
                throw std::runtime_error(SDL_GetError());
            }
        }
        ~Core(){
            SDL_Quit();
        }
    };
    // SDL_IMG初期化
    struct Img{    
        Img(){
            if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
                throw std::runtime_error(IMG_GetError());
            }
        }
        ~Img(){
            IMG_Quit();
        }
    };
    // SDL_TTF初期化
    struct Ttf{
        Ttf(){
            if((TTF_Init() != 0)){
                throw std::runtime_error(TTF_GetError());
            }
        }
        ~Ttf(){
            TTF_Quit();
        }
    };
    // SDL_Mixer初期化
    // Init
    struct MixerCore{    
        MixerCore(){
            // mp3を使う場合はMIX_INIT_MP3が必要
            int mixFlags = MIX_INIT_OGG;
            if((Mix_Init(mixFlags) & mixFlags) != mixFlags) {
                throw std::runtime_error(Mix_GetError());
            }
        }
        ~MixerCore(){
            Mix_Quit();
        }
    };
    // Device
    struct MixerDevice{    
        MixerDevice(){
            // 44100Hz, ステレオ, バッファ2048サンプル(mp3を使う場合はMIX_INIT_MP3が必要)
            if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
                throw std::runtime_error(Mix_GetError());
            }
        }
        ~MixerDevice(){
            Mix_CloseAudio();   // Mixerデバイスの終了
        }
    };


    // RAII
    Core        core;       // 1番目に初期化，5番目に破棄
    Img         img;        // 2番目に初期化，4番目に破棄
    Ttf         ttf;        // 3番目に初期化，3番目に破棄
    MixerCore   mixerCore;  // 4番目に初期化，2番目に破棄
    MixerDevice mixerDevice;// 5番目に初期化，1番目に破棄
    
    // 二重解放防止用のコンストラクタ宣言
    SdlSystem() = default;
    ~SdlSystem() = default;
    // コピー禁止
    SdlSystem(const SdlSystem&) = delete;
    SdlSystem& operator=(const SdlSystem&) = delete;
};

#endif  // PLATFORM_SDLCONTEXT_H_
