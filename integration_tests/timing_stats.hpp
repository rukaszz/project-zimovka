#ifndef ZIMOVKA_INTEGRATION_TESTS_TIMING_STATS_HPP_
#define ZIMOVKA_INTEGRATION_TESTS_TIMING_STATS_HPP_

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <string>
#include <vector>

namespace test_util{
/**
 * @brief 処理時間サンプル列からavg/p95/p99/maxを計算するユーティリティ
 * 
 * avg：平均
 * p95：95%が
 */
struct TimingStats{
    std::int64_t count  = 0;
    std::int64_t avg_ns = 0;
    std::int64_t p95_ns = 0;
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
        // 引数チェック
        if(samples.empty()){
            return {};
        }
        // サンプル群をソート
        std::sort(samples.begin(), samples.end());
        
        TimingStats s;
        s.count = static_cast<std::int64_t>(samples.size());
        // 計算(sum, avg)
        // [first, last)で畳み込み(デフォルトは加算+なので合計値が求まる), 初期値ゼロ
        const std::int64_t sum = std::accumulate(
            samples.begin(), samples.end(), std::int64_t{0}
        );
        s.avg_ns = sum / s.count;
        
        // パーセンタイル関数(samplesはソート済みである必要がある)
        auto pct = [&](double p) -> std::int64_t {
            // サンプリング用インデックスの計算
            const std::size_t idx = static_cast<std::size_t>(
                // (size - 1)が最大インデックス，size_tなので0.5を加算して四捨五入する
                p / 100.0 * static_cast<double>(samples.size() - 1) + 0.5
            );
            // idxが範囲外にならないようにclampして確率pの位置にあるサンプルを返す
            return samples[std::min(idx, samples.size() - 1u)];
        };
        s.p95_ns = pct(95.0);
        s.p99_ns = pct(99.0);
        s.max_ns = samples.back();  // ソート済みなので最大値

        return s;
    }
};

/**
 * @brief タイミングの統計値をCSVファイルに追記する
 *
 * プロセス内で最初に呼ばれたときはファイルを新規作成してヘッダ行を書き込み，
 * 以降の呼び出しは同じファイルに追記する．
 * ディレクトリが存在しない場合は自動で作成する．
 *
 * p95, p99は上位X%の値→遅い処理のみなす境界値
 * 
 * CSV列: suite,test,count,avg_us,p95_us,p99_us,max_us
 *        テストスイート，テスト名，Tick数，p95の値，p99の値，最大値
 * 
 * @param path      出力ファイルパス (例: "bench_results/timing.csv")
 * @param suite     テストスイート名
 * @param test_name テスト(またはサブ項目)の名前
 * @param s         書き込む統計値
 */
inline void AppendTimingCSV(
    const std::string& path,
    const std::string& suite,
    const std::string& test_name,
    const TimingStats& s
){
    namespace fs = std::filesystem;

    // 初回呼び出し時はtrunc(上書き)でヘッダ行を書き，以降はappend
    static bool header_written = false;
    // 親ディレクトリbench_resultsの存在チェック
    const auto parent = fs::path(path).parent_path();
    if(!parent.empty()){
        fs::create_directories(parent);
    }
    // trunc/appendの切り替え
    // 書き込み用openフラグoutとモード(trunc/app)をビット演算で指定する
    const auto mode = header_written
        ? (std::ios::out | std::ios::app)       
        : (std::ios::out | std::ios::trunc);

    std::ofstream f(path, mode);
    // ファイルオープンに失敗したら何もしない
    if(!f){
        return;
    }

    if(!header_written){
        f << "suite,test,count,avg_us,p95_us,p99_us,max_us\n";
        header_written = true;
    }
    f << suite     << ','
      << test_name << ','
      << s.count   << ','
      << s.avg_ns / 1000 << ','
      << s.p95_ns / 1000 << ','
      << s.p99_ns / 1000 << ','
      << s.max_ns / 1000 << '\n';
}

}   // namespace test_util

#endif  // ZIMOVKA_INTEGRATION_TESTS_TIMING_STATS_HPP_
