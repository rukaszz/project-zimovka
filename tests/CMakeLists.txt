# GoogleTestをFetchContentで取得(システムを汚染しない)
include(FetchContent)
# GithubからGoogleTestをダウンロードしてビルド対象に取り込む
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Windows向け: ランタイムDLLの設定を統一する
# DDL版C Runtimeを強制し，親プロジェクトのコンパイラ・リンカの設定を上書きするのを防ぐ
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ──────────────────────────────────────────────────────
# テスト対象ソース
# main.cpp/SdlContext/Window/Renderer/DebugOverlayなど
# SDL初期化が必要なファイルは除外する
# PrimitiveRenderer.cppのリンクは
# BulletSystem.cppとPlayerSystem.cppがPrimitiveRenderer.hppを
# インクルードするため含めている
# ──────────────────────────────────────────────────────
set(ZIMOVKA_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/input/InputState.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/bullet/BulletSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/player/PlayerSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/player/PlayerWeaponSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/collision/CollisionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/PrimitiveRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/replay/RunRecorder.cpp
)

# ビルドする実行ファイル設定
add_executable(zimovka_tests
    test_Vec2.cpp
    test_InputState.cpp
    test_InputSystem.cpp
    test_BulletSystem.cpp
    test_PlayerSystem.cpp
    test_Collision.cpp
    test_DebugAccumulator.cpp
    test_RunRecorder.cpp
    test_PlayerWeaponSystem.cpp
    ${ZIMOVKA_TEST_SOURCES}
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
