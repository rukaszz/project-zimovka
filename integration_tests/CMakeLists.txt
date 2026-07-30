# ──────────────────────────────────────────────────────
# 複合試験(Integration/Performance Tests) - フェーズ0
# ※GoogleTestはルートディレクトリのCMakeLists.txtでFetchContent済み
# ──────────────────────────────────────────────────────

# 複合試験で使用するソースファイル
# SDL初期化が必要なファイル・main.cppは除外
set(ZIMOVKA_INTEGRATION_SOURCES
    ${CMAKE_SOURCE_DIR}/src/systems/bullet/BulletSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/enemy/EnemySystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/collision/CollisionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/PrimitiveRenderer.cpp
)
# 定常負荷試験ソースの指定
add_executable(zimovka_integration_tests
    test_perf_steady_phase0.cpp
    ${ZIMOVKA_INTEGRATION_SOURCES}
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
