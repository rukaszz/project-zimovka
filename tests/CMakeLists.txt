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
    test_PatternSystem.cpp
    test_PlayerBombSystem.cpp
    test_PlayerSystem.cpp
    test_Collision.cpp
    test_DebugAccumulator.cpp
    test_RunRecorder.cpp
    test_PlayerWeaponSystem.cpp
    test_DeterministicRng.cpp
    test_Texture.cpp
    test_GameplayMath.cpp
    ${ZIMOVKA_CORE_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/input/InputSystem.cpp
    # Texture は APP_SOURCES だが，Load()の引数チェック(null renderer)をテストするために追加
    # リンクに SDL2IMAGE が必要
    ${CMAKE_SOURCE_DIR}/src/rendering/texture/Texture.cpp
)

# インクルードパス(親CMakeLists.txtでpkg_check_modulesが実行済みなので変数が使える)
target_include_directories(zimovka_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

# リンク: GTest + SDL2(PrimitiveRenderer.cpp/InputSystem.cpp) + SDL2IMAGE(Texture.cpp)
target_link_libraries(zimovka_tests PRIVATE
    GTest::gtest_main
    ${SDL2_LIBRARIES}
    ${SDL2IMAGE_LIBRARIES}
)

target_compile_options(zimovka_tests PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# テスト用一時画像の生成・削除先ディレクトリをコンパイル定義として渡す
target_compile_definitions(zimovka_tests PRIVATE
    ZIMOVKA_TEST_IMAGE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests_image"
)

# CTestへ自動登録
include(GoogleTest)
gtest_discover_tests(zimovka_tests)
