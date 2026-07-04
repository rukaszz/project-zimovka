#include "zimovka/systems/bullet/BulletSystem.hpp"

#include "zimovka/rendering/PrimitiveRenderer.hpp"

namespace zimovka{

/**
 * @brief Construct a new Bullet System:: Bullet System object
 *
 * ゲーム開始時にmax_bullets個分のメモリを一括確保する．
 * 各要素はBullet{}で初期化される(active = false)
 *
 * @param max_bullets プール最大数
 */
BulletSystem::BulletSystem(std::size_t max_bullets)
    : next_spawn_idx_(0)
{
    // resize: 要素をBullet{}(空)で埋めつつ連続メモリを確保
    // resizeを用いるとそのサイズ分コンストラクタが走るので注意
    bullets_.resize(max_bullets);
}

/**
 * @brief 弾を生成する関数(inactive → activeの状態遷移)
 *
 * next_spawn_idx_から始めて循環しながら空(!active)スロットを探す
 * 弾は生成された順に期限切れになる可能性が高い(傾向がある)
 * →よって，循環インデックスにより平均的にO(1)で空きを見つけられる
 *
 * @param position  初期座標
 * @param velocity  速度 (px/s)
 * @param radius    弾半径
 * @param color     弾色
 * @return true 生成成功 / false プール満杯
 */
bool BulletSystem::Spawn(
    const Vec2& position, const Vec2& velocity,
    float radius, Color color
)
{
    // 無効な半径はfalse
    if (radius <= 0.0f) {
        return false;
    }
    // サイズ取得
    const std::size_t size = bullets_.size();
    // サイズ分連番で走査
    for(std::size_t i = 0; i < size; ++i){
        // 次に出現するインデックスを設定(0〜最大弾数の周期で回る)
        const std::size_t idx = (next_spawn_idx_ + i) % size;
        // inactiveな弾を発見したら表示
        if(!bullets_[idx].active){
            bullets_[idx].active   = true;
            bullets_[idx].position = position;
            bullets_[idx].velocity = velocity;
            bullets_[idx].radius   = radius;
            bullets_[idx].color    = color;
            // 次のSpawn()は1つ後のスロットから探す
            next_spawn_idx_ = (idx + 1) % size;
            return true;
        }
    }
    // 全配列を走査してinactiveがゼロ=プールが満杯
    return false;
}

/**
 * @brief 更新処理
 *
 * activeな弾の位置をvelocity * dt で更新し，
 * 完全に画面外へ出たらinactiveにする
 *
 * @param dt            固定デルタ時間 (s)
 * @param screen_width  画面幅 (px)
 * @param screen_height 画面高さ (px)
 */
void BulletSystem::Update(float dt, float screen_width, float screen_height){
    // bullet走査
    for(auto& b : bullets_){
        // inactiveは飛ばす
        if(!b.active){
            continue;
        }
        // 位置更新
        b.position += b.velocity * dt;
        // 画面外に出たら非活性化
        if(IsOutOfScreen(b, screen_width, screen_height)){
            b.active = false;
        }
    }
}

/**
 * @brief 描画処理
 *
 * activeな弾のみ描く
 *
 * @param renderer PrimitiveRenderer への参照
 */
void BulletSystem::Render(PrimitiveRenderer& renderer) const{
    // bullet線形全探索
    for(const auto& b : bullets_){
        // inactiveは飛ばす
        if(!b.active){
            continue;
        }
        // activeは描画
        renderer.DrawFilledCircle(
            b.position.x,
            b.position.y,
            static_cast<int>(b.radius),
            b.color
        );
    }
}

/**
 * @brief 弾を全部消去する関数
 * 
 * 配列を消すのではなくすべての弾を非活性にする
 */
void BulletSystem::Clear() noexcept{
    // 全ての弾をなめる
    for(auto& b : bullets_){
        b.active = false;
    }
    // スポーンインデックスも初期化
    next_spawn_idx_ = 0;
}

/**
 * @brief active な弾の数を返す(Debug用)
 * 弾数プールの上限に応じてactive_count_を検討
 *
 * @return std::size_t
 */
std::size_t BulletSystem::CountActive() const noexcept{
    // 戻り値
    std::size_t count = 0;
    for(const auto& b : bullets_){
        // 単純な計測
        if(b.active){
            ++count;
        }
    }
    return count;
}

/**
 * @brief 弾が画面外に完全に出たかどうかを判定する
 *
 * 弾の半径を考慮して，弾の端が画面端を超えたときにtrueを返す
 * XまたはYのどちらかが画面外ならtrue
 *
 * @param bullet        判定対象の弾
 * @param screen_width  画面幅 (px)
 * @param screen_height 画面高さ (px)
 * @return true 完全に画面外 / false 画面内
 */
bool BulletSystem::IsOutOfScreen(const Bullet& bullet, float screen_width, float screen_height) const{
    // 半径を加味して上下左右を調べる
    return (bullet.position.x + bullet.radius < 0.0f
         || bullet.position.x - bullet.radius > screen_width
         || bullet.position.y + bullet.radius < 0.0f
         || bullet.position.y - bullet.radius > screen_height);
}

}   // namespace zimovka
