#include "zimovka/rendering/SpriteRenderer.hpp"

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
    // dstがfloatなのでFを仕様
    SDL_RenderCopyExF(
        renderer_,
        texture.Get(),
        nullptr,        // src rect: nullptr = テクスチャ全体
        &dst,
        params.angle_deg,
        nullptr,        // center: nullptr = dst矩形の中心
        flip
    );
}

} // namespace zimovka
