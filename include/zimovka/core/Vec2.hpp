#ifndef ZIMOVKA_CORE_VEC2_HPP_
#define ZIMOVKA_CORE_VEC2_HPP_

#include <cmath>
#include <istream>
#include <ostream>

namespace zimovka{

/**
 * @brief zimovka独自の2次元ベクトルを取り扱う構造体Vec2
 * float型固定でとりあえず定義
 * ※整数値やdouble型が必要になった場合にテンプレート化を検討
 */
struct Vec2{
    float x = 0.0f;
    float y = 0.0f;

    // デフォルトコンストラクタ
    Vec2() = default;
    
    // 初期値を与えられるコンストラクタ
    constexpr Vec2(float x_, float y_)
        : x(x_), y(y_) {}
    
    // メンバ関数
    /**
     * @brief 2次元ベクトルの長さ(ノルム)
     * 
     * @return float 
     */
    float Length() const{
        return std::sqrt(LengthSquared());
    }
    /**
     * @brief 2次元ベクトルの長さの2乗
     * 
     * @return float 
     */
    float LengthSquared() const{
        return Dot(*this);
    }
    /**
     * @brief 2次元ベクトル同士の内積
     * 
     * @param other 
     * @return float 
     */
    float Dot(const Vec2& other) const{
        return x * other.x + y * other.y;
    }
    /**
     * @brief 2次元ベクトル同士の距離
     * 
     * @param other 
     * @return float 
     */
    float DistanceFrom(const Vec2& other) const{
        // 等価な計算式：std::sqrt((other.x - x)*(other.x - x) + (other.y - y)*(other.y - y));
        return (other - *this).Length();
    }
    /**
     * @brief 当該2次元ベクトルの正規化
     * 
     * 長さを1としたベクトルが返る
     * 
     * @return Vec2 
     */
    Vec2 Normalized() const{
        // ゼロ除算チェックは必要
        const float len = Length();
        return (len > 0.0f) ? *this/len : Vec2{0.0f, 0.0f};
    }
    /**
     * @brief ゼロベクトルかを返す
     * 
     * @return true 
     * @return false 
     */
    bool IsZero() const{
        return (x == 0.0f && y == 0.0f);
    }

    // 演算子オーバロード
    /**
     * @brief 単項の+
     * 
     * @return Vec2 
     */
    Vec2 operator+() const{
        return *this;
    }
    /**
     * @brief 単項の-
     * 
     * @return Vec2 
     */
    Vec2 operator-() const{
        return {-x, -y};
    }
    /**
     * @brief 2つの2次元ベクトルの加法定義
     * 
     * @param other 
     * @return Vec2 
     */
    Vec2 operator+(const Vec2& other) const{
        return {x+other.x, y+other.y};
    }
    /**
     * @brief 2つの2次元ベクトルの減法定義
     * 
     * @param other 
     * @return Vec2 
     */
    Vec2 operator-(const Vec2& other) const{
        return {x-other.x, y-other.y};
    }
    /**
     * @brief 2次元ベクトルのスカラ積
     * 
     * @param k 
     * @return Vec2 
     */
    Vec2 operator*(float k) const{
        return {x*k, y*k};
    }
    /**
     * @brief 2次元ベクトルのスカラ商
     * 
     * @param k 
     * @return Vec2 
     */
    Vec2 operator/(float k) const{
        return {x/k, y/k};
    }
    // 複合代入演算子(自己代入なので非const)
    /**
     * @brief 2次元ベクトル同士の複合代入演算子(+=)
     * 
     * @param other 
     * @return Vec2 
     */
    Vec2& operator+=(const Vec2& other){
        x += other.x;
        y += other.y;
        return *this;
    }
    /**
     * @brief 2次元ベクトル同士の複合代入演算子(-=)
     * 
     * @param other 
     * @return Vec2 
     */
    Vec2& operator-=(const Vec2& other){
        x -= other.x;
        y -= other.y;
        return *this;
    }
    /**
     * @brief 2次元ベクトル同士の複合代入演算子(*=)
     * 
     * @param other 
     * @return Vec2 
     */
    Vec2& operator*=(float k){
        x *= k;
        y *= k;
        return *this;
    }
    /**
     * @brief 2次元ベクトル同士の複合代入演算子(/=)
     * 
     * @param other 
     * @return Vec2 
     */
    Vec2& operator/=(float k){
        x /= k;
        y /= k;
        return *this;
    }
    // 入出力ストリームの"<<"と">>"をオーバロード
    // <<
    template <class Char, class Traits>
    friend std::basic_ostream<Char, Traits>&
    operator<<(std::basic_ostream<Char, Traits>& os, const Vec2& v);
    // >>
    template <class Char, class Traits>
    friend std::basic_istream<Char, Traits>&
    operator>>(std::basic_istream<Char, Traits>& is, Vec2& v);
};

// 左辺がスカラの二項演算
/**
 * @brief 左オペランドが定数なのでメンバにできない
 * 同ファイルに置くだけでADLが見つけられる(using不要)
 * Vec2の振る舞いではなく，外部から見たVec2の振る舞いの定義
 * なのでfriendを用いて外部から非メンバのVec2へアクセスできるようにしている
 * 
 * @param k 
 * @param v 
 * @return Vec2 
 */
inline Vec2 operator*(float k, const Vec2& v){
    return {k*v.x, k*v.y};
}

// 出力演算子オーバーロード
template <class Char, class Traits>
inline std::basic_ostream<Char, Traits>&
operator<<(std::basic_ostream<Char, Traits>& os, const Vec2& v){
    // static_cast<Char>('(')は文字のchar型キャスト
    return os << static_cast<Char>('(') << v.x << static_cast<Char>(',') << v.y << static_cast<Char>(')');
}

// 入力演算子オーバーロード
template <class Char, class Traits>
inline std::basic_istream<Char, Traits>&
operator>>(std::basic_istream<Char, Traits>& is, Vec2& v){
    // 入力の受け入れ
    Char ch;
    // (x,y)形式の入力のみ受け入れる
    // '('
    if(!(is >> ch) || ch != static_cast<Char>('(')){
        is.setstate(std::ios_base::failbit);
        return is;
    }
    // x
    if(!(is >> v.x)){
        return is;
    }
    // ','
    if(!(is >> ch) || ch != static_cast<Char>(',')){
        is.setstate(std::ios_base::failbit);
        return is;
    }
    // y
    if(!(is >> v.y)){
        return is;
    }
    // ')'
    if(!(is >> ch) || ch != static_cast<Char>(')')){
        is.setstate(std::ios_base::failbit);
        return is;
    }
    return is;
}

}   // namespace zimovka

#endif  // ZIMOVKA_CORE_VEC2_HPP_
