# ──────────────────────────────────────────────────────
# 複合試験(Integration Tests)
# ※GoogleTestはルートディレクトリのCMakeLists.txtでFetchContent済み
# ──────────────────────────────────────────────────────
# ZIMOVKA_CORE_SOURCES/ZIMOVKA_ENGINE_SOURCES は
# ルートCMakeLists.txtで定義済み→ここでは参照するだけ
#
# RunPlayback.cppはまだ本体に含まれていないため
# 複合試験ターゲットのみに個別追加する
# ──────────────────────────────────────────────────────

# 複合試験ソースの指定
add_executable(zimovka_integration_tests
    test_phase0_steady_integration.cpp
    test_phase0_churn_integration.cpp
    test_phase0_gamelike_integration.cpp
    ${ZIMOVKA_CORE_SOURCES}
    ${ZIMOVKA_ENGINE_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/replay/RunPlayback.cpp
)
# 必要なヘッダ読み込み
target_include_directories(zimovka_integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)
# gtest/SDL2のライブラリ読み込み
target_link_libraries(zimovka_integration_tests PRIVATE
    GTest::gtest_main
    ${SDL2_LIBRARIES}
)
# オプション指定
target_compile_options(zimovka_integration_tests PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)
# gtest設定
include(GoogleTest)
gtest_discover_tests(zimovka_integration_tests)

# ──────────────────────────────────────────────────────
# build command
# $ cmake -B build -DBUILD_INTEGRATION_TESTING=ON
# $ cmake --build build --target zimovka_integration_tests
# $ ./bin/zimovka_integration_tests
# ──────────────────────────────────────────────────────
