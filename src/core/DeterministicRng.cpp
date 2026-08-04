#include "zimovka/core/DeterministicRng.hpp"

#include <cmath>
#include <stdexcept>

namespace zimovka{

/**
 * @brief 暗黙的な型変換を防止したコンストラクタ
 * 
 * シード値をメルセンヌツイスタへ与える
 * 
 * @param seed 
 */
DeterministicRng::DeterministicRng(Seed seed) noexcept
    : mt_engine_(seed)
{

}

/**
 * @brief メルセンヌツイスタへシード値を設定する
 * 
 * @param seed 
 */
void DeterministicRng::SeedEngine(Seed seed) noexcept{
    mt_engine_.seed(seed);
}

/**
 * @brief uint32型の乱数を返す
 * 
 * @return std::uint32_t 
 */
std::uint32_t DeterministicRng::NextU32() noexcept{
    return static_cast<std::uint32_t>(mt_engine_());
}
// 

/**
 * @brief 偏りを出さずに範囲内のランダムな整数値(uint32_t)を出す
 * 
 * サンプリングは棄却サンプリング法を使っている
 * 
 * @param min 
 * @param max 
 * @return std::uint32_t 
 */
std::uint32_t DeterministicRng::UniformU32(
    std::uint32_t min, std::uint32_t max
)
{
    // 引数の整合性チェック
    if(min > max){
        throw std::invalid_argument(
            "UniformU32 requires min <= max. "
        );
    }
    // 乱数のとり得る値の領域サイズ(uint64_tを32ビット左シフトして作成)：2^32
    constexpr std::uint64_t DOMAIN_SIZE = std::uint64_t{1} << 32;
    // 乱数がとり得る値の範囲，要素数(0〜10なら，集合の要素数は11)
    const std::uint64_t range = static_cast<std::uint64_t>(max)
        - static_cast<std::uint64_t>(min) 
        + 1u;
    // rangeの範囲に一様に落とすために，棄却する上端の境界値の設定
    const std::uint64_t limit = DOMAIN_SIZE - (DOMAIN_SIZE % range);
    // 乱数の値
    std::uint32_t value = 0;
    // 棄却サンプリング法(limitを超えた場合は再度サンプリング)
    do{
        value = NextU32();
    } while (
        static_cast<std::uint64_t>(value) >= limit
    );
    // max/minの範囲内へ収めるオフセット
    const std::uint64_t offset = static_cast<std::uint64_t>(value) % range;
    
    return static_cast<uint32_t>(
        static_cast<std::uint64_t>(min) + offset
    );
}

/**
 * @brief [0, 1)のfloat乱数値の生成
 * 
 * @return float 
 */
float DeterministicRng::UnitFloat() noexcept{
    // float型を[0, 1)の範囲に収める
    constexpr float SCALE = 1.0f / 16'777'216.0f;   // 1 / 2^64
    // 8桁右シフトして切り捨てた値をSCALEで[0, 1)に収める
    return static_cast<float>(
        NextU32() >> 8  // floatは仮数部23bitであり，2^24以上は表現できないので切り捨て
    ) * SCALE;
}

/**
 * @brief 偏りを出さずに引数の範囲内のfloat型乱数値へ変換する
 * 
 * @param min 
 * @param max 
 * @return float 
 */
float DeterministicRng::UniformFloat(float min, float max){
    // 引数の整合性チェック
    if(!std::isfinite(min)
    || !std::isfinite(max)
    || min > max)
    {
        throw std::invalid_argument(
            "UniformFloat received invalid range. "
        );
    }
    // max/min範囲内に収めつつfloat型の乱数を作る
    return min + (max - min) * UnitFloat();
}

}   // namespace zimovka
