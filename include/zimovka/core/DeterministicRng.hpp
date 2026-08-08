#ifndef ZIMOVKA_CORE_DETERMINISTICRNG_HPP_
#define ZIMOVKA_CORE_DETERMINISTICRNG_HPP_

#include <cstdint>
#include <random>

namespace zimovka{
/**
 * @brief 決定論的乱数管理クラス
 * 
 * リプレイなどでも使える乱数でありseed値によって同一の結果となる
 * この乱数はゲームロジックに用いる
 * 
 */
class DeterministicRng{
public:
    // シード値
    using Seed = std::uint32_t;
    // 暗黙的な型変換を防止
    explicit DeterministicRng(Seed seed = 0) noexcept;

    // コピー禁止：同一値コピーによる意図しない乱数の重複を防止
    DeterministicRng(const DeterministicRng&) = delete;
    DeterministicRng& operator=(const DeterministicRng&) = delete;
    // ムーブ許可：単純な所有権移動なので許可
    DeterministicRng(DeterministicRng&&) noexcept = default;
    DeterministicRng& operator=(DeterministicRng&&) noexcept = default;

    // シード値設定
    void Reseed(Seed seed) noexcept;
    // 次に出る値(uint32_t)
    std::uint32_t NextU32() noexcept;
    // 偏りを出さずに範囲内の整数値へ変換する
    std::uint32_t UniformU32(std::uint32_t min, std::uint32_t max);
    // [0, 1)のfloat乱数値の生成
    float UnitFloat() noexcept;
    // 偏りを出さずに範囲内のfloat値へ変換する
    float UniformFloat(float min, float max);

private:
    // 乱数の証跡(同一seedで乱数の消費のずれなどをチェックする)
    // 乱数の消費回数
    std::uint64_t draw_count_ = 0;
    // 現在のシード値
    Seed current_seed_ = 0;
    // メルセンヌツイスタ乱数生成器(32ビット版)
    std::mt19937 mt_engine_;
};

}   // namespace zimovka

#endif  // ZIMOVKA_CORE_DETERMINISTICRNG_HPP_
