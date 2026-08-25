# ──────────────────────────────────────────────────────
# 単体試験(Unit Tests)
# GoogleTestはルートCMakeLists.txtでFetchContent済み
# ──────────────────────────────────────────────────────
# ZIMOVKA_CORE_SOURCESはルートCMakeLists.txtで定義済み
# (SDL不要なゲームシステム群は参照するだけ)
#
# InputSystem.cpp はSDL events依存でCOREに含めていないため
# unit testターゲットのみに個別追加する
# ──────────────────────────────────────────────────────

# ビルドする実行ファイル設定
add_executable(zimovka_tests
    test_Vec2.cpp
    test_InputState.cpp
    test_InputSystem.cpp
    test_BulletSystem.cpp
    test_EnemySystem.cpp
    test_PlayerSystem.cpp
    test_Collision.cpp
    test_DebugAccumulator.cpp
    test_RunRecorder.cpp
    test_PlayerWeaponSystem.cpp
    test_DeterministicRng.cpp
    ${ZIMOVKA_CORE_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/input/InputSystem.cpp
)

# インクルードパス(親CMakeLists.txtでpkg_check_modulesが実行済みなので変数が使える)
target_include_directories(zimovka_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

# リンク: GTest + SDL2(PrimitiveRenderer.cppとInputSystem.cppで使用する)
target_link_libraries(zimovka_tests PRIVATE
    GTest::gtest_main
    ${SDL2_LIBRARIES}
)

target_compile_options(zimovka_tests PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# CTestへ自動登録
include(GoogleTest)
gtest_discover_tests(zimovka_tests)
