# ──────────────────────────────────────────────────────
# 複合試験(Integration Tests) - フェーズ0
# ※GoogleTestはルートディレクトリのCMakeLists.txtでFetchContent済み
# ──────────────────────────────────────────────────────

# 複合試験で使用するソースファイル
# SDL初期化が必要なファイル・main.cppは除外
set(ZIMOVKA_INTEGRATION_SOURCES
    ${CMAKE_SOURCE_DIR}/src/systems/bullet/BulletSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/enemy/EnemySystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/collision/CollisionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/PrimitiveRenderer.cpp
    # GameLike複合試験用 (UpdatePipeline依存)
    ${CMAKE_SOURCE_DIR}/src/engine/update/UpdatePipeline.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/player/PlayerSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/player/PlayerWeaponSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputState.cpp
    ${CMAKE_SOURCE_DIR}/src/replay/RunRecorder.cpp
    ${CMAKE_SOURCE_DIR}/src/replay/RunPlayback.cpp
    ${CMAKE_SOURCE_DIR}/src/core/DeterministicRng.cpp
)
# 複合試験ソースの指定
add_executable(zimovka_integration_tests
    test_phase0_steady_integration.cpp
    test_phase0_churn_integration.cpp
    test_phase0_gamelike_integration.cpp
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
