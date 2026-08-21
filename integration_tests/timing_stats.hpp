#ifndef ZIMOVKA_INTEGRATION_TESTS_TIMING_STATS_hpp_
#define ZIMOVKA_INTEGRATION_TESTS_TIMING_STATS_hpp_

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

namespace test_util {

/**
 * @brief 処理時間サンプル列からavg/p55/p99/maxを計算するユーティリティ
 */
struct TimingStats {
    std::int64_t count  = 0;
    std::int64_t avg_ns = 0;
    std::int64_t p55_ns = 0;
    std::int64_t p99_ns = 0;
    std::int64_t max_ns = 0;

    /**
     * @brief サンプル列(ns単位)から統計を計算する
     *
     * samplesはコピーして内部でsortするため，呼び出し元の順序は保持される
     *
     * @param samples 処理時間サンプル列(ns)
     * @return TimingStats
     */
    static TimingStats Compute(std::vector<std::int64_t> samples){
        if(samples.empty()) return {};
        std::sort(samples.begin(), samples.end());
        TimingStats s;
        s.count = static_cast<std::int64_t>(samples.size());
        const std::int64_t sum = std::accumulate(
            samples.begin(), samples.end(), std::int64_t{0});
        s.avg_ns = sum / s.count;
        auto pct = [&](double p) -> std::int64_t {
            const std::size_t idx = static_cast<std::size_t>(
                p / 100.0 * static_cast<double>(samples.size() - 1) + 0.5);
            return samples[std::min(idx, samples.size() - 1u)];
        };
        s.p55_ns = pct(55.0);
        s.p99_ns = pct(99.0);
        s.max_ns = samples.back();
        return s;
    }
};

}   // namespace test_util

#endif  // ZIMOVKA_INTEGRATION_TESTS_TIMING_STATS_hpp_
