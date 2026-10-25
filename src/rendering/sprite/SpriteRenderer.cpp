#include "zimovka/rendering/sprite/SpriteRenderer.hpp"

namespace zimovka{

/**
 * @brief テクスチャをスプライトとして描画する
 *
 * 中心座標を基準にsize分の矩形を描画
 * SDL_RenderCopyExを利用するため回転・反転が可能
 *
 * ※Playerなどと同様に座標は中心(center)で管理
 * @param texture  描画するテクスチャ
 * @param params   描画パラメータ(中心座標・サイズ・回転・反転)
 */
void SpriteRenderer::Draw(
    const Texture& texture,
    const SpriteDrawParams& params
)
{
    // 描画用矩形の構成※FRectはfloat
    const SDL_FRect dst{
        params.center.x - params.size.x * 0.5f,
        params.center.y - params.size.y * 0.5f,
        params.size.x,
        params.size.y,
    };
    // flip_xがtrueで反転
    const SDL_RendererFlip flip =
        params.flip_x ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    // alpha < 255 の場合にアルファモジュレーションを適用
    // デフォルトは255なので255の場合は何もしない
    const bool use_alpha = (params.alpha != 255);
    if(use_alpha){
        SDL_SetTextureBlendMode(texture.Get(), SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(texture.Get(), params.alpha);
    }
    // dstがfloatなのでFを使用
    SDL_RenderCopyExF(
        renderer_,
        texture.Get(),
        nullptr,        // src rect: nullptr = テクスチャ全体
        &dst,
        params.angle_deg,
        nullptr,        // center: nullptr = dst矩形の中心
        flip
    );
    // 他の描画に影響させないようにalphaを255に復元
    if(use_alpha){
        SDL_SetTextureAlphaMod(texture.Get(), 255);
    }
}

} // namespace zimovka
