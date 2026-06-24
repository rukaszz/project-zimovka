#include "zimovka/platform/SdlContext.hpp"

#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

/**
 * @brief Construct a new zimovka::Sdl Context::Core::Core object
 * 
 * SDL本体の初期化
 */
zimovka::SdlContext::Core::Core(){
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0){
        throw std::runtime_error(SDL_GetError());
    }
}

/**
 * @brief Destroy the zimovka::Sdl Context::Core::Core object
 * 
 * SDL本体の破棄
 */
zimovka::SdlContext::Core::~Core(){
    SDL_Quit();
}

/**
 * @brief Construct a new zimovka::Sdl Context::Img::Img object
 * 
 * SDL_imageの初期化
 */
zimovka::SdlContext::Img::Img(){
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
        throw std::runtime_error(IMG_GetError());
    }
}

/**
 * @brief Destroy the zimovka::Sdl Context::Img::Img object
 * 
 * SDL_imageの破棄
 */
zimovka::SdlContext::Img::~Img(){
    IMG_Quit();
}

/**
 * @brief Construct a new zimovka::Sdl Context::Ttf::Ttf object
 * 
 * SDL_ttf初期化
 */
zimovka::SdlContext::Ttf::Ttf(){
    if(TTF_Init() != 0){
        throw std::runtime_error(TTF_GetError());
    }
}

/**
 * @brief Destroy the zimovka::Sdl Context::Ttf::Ttf object
 * 
 * SDL_ttfの破棄
 */
zimovka::SdlContext::Ttf::~Ttf(){
    TTF_Quit();
}

/**
 * @brief Construct a new zimovka::Sdl Context::Mixer Core::Mixer Core object
 * 
 * MixerCore(SDL_mixer コーデック初期化)
 */
zimovka::SdlContext::MixerCore::MixerCore(){
    // mp3を使う場合はMIX_INIT_MP3が必要
    int mixFlags = MIX_INIT_OGG;
    if((Mix_Init(mixFlags) & mixFlags) != mixFlags){
        throw std::runtime_error(Mix_GetError());
    }
}

/**
 * @brief Destroy the zimovka::Sdl Context::Mixer Core::Mixer Core object
 * 
 * MixerCore(SDL_mixer コーデック破棄)
 */
zimovka::SdlContext::MixerCore::~MixerCore(){
    Mix_Quit();
}

/**
 * @brief Construct a new zimovka::Sdl Context::Mixer Device::Mixer Device object
 * 
 * MixerDevice(SDL_mixer デバイスオープン)
 */
zimovka::SdlContext::MixerDevice::MixerDevice(){
    // 44100Hz, ステレオ, バッファ2048サンプル
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
        throw std::runtime_error(Mix_GetError());
    }
}

/**
 * @brief Destroy the zimovka::Sdl Context::Mixer Device::Mixer Device object
 * 
 * MixerDevice (SDL_mixer デバイスクローズ)
 */
zimovka::SdlContext::MixerDevice::~MixerDevice(){
    Mix_CloseAudio();
}
