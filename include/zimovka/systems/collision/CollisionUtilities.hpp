#ifndef ZIMOVKA_SYSTEMS_COLLISION_COLLISIONUTILITIES_HPP_
#define ZIMOVKA_SYSTEMS_COLLISION_COLLISIONUTILITIES_HPP_

#include "zimovka/core/Circle.hpp"
#include "zimovka/core/Vec2.hpp"

/**
 * @brief CollisionSystemで使用する接触判定用関数
 * テストでエッジケース等を確認する都合上，別のヘッダファイルに定義を逃がす
 * 
 */
namespace zimovka{
namespace CollisionUtilities{
/**
 * @brief 円aと円bの交差判定用関数
 * すり抜け(トンネリング)を考慮しない簡素な関数
 * この関数を用いる場合は，円同士の"接触(<=)"は被弾である
 * 
 * @param a 
 * @param b 
 * @return true 
 * @return false 
 */
inline bool Intersects(const Circle& a, const Circle& b) noexcept{
    // 円の中心座標の差
    const Vec2 d = a.center - b.center;
    // 円の半径の和
    const float r = a.radius + b.radius;
    /**
     * @brief d <= r1 + r2 
     * 2つの円の中心座標の差d が 2つの円の半径の和より小さいなら円は交差している
     * なお，d^2 <= (r1 + r2)^2 にすると平方根が無視できる
     */
    return d.LengthSquared() <= r * r;
}

}   // namespace CollisionUtilities
}   // namespace zimovka

#endif  // ZIMOVKA_SYSTEMS_COLLISION_COLLISIONUTILITIES_HPP_
