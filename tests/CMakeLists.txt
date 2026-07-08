# GoogleTestをFetchContentで取得(システムを汚染しない)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Windows向け: ランタイムDLLの設定を統一する
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ──────────────────────────────────────────────────────
# テスト対象ソース
# main.cpp / SdlContext / Window / Renderer / DebugOverlay など
# SDL初期化が必要なファイルは除外する
# BulletSystem.cpp と PlayerSystem.cpp は PrimitiveRenderer.hpp を
# インクルードするため，PrimitiveRenderer.cpp のリンクが必要
# ──────────────────────────────────────────────────────
set(ZIMOVKA_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/input/InputState.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/bullet/BulletSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/player/PlayerSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/collision/CollisionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/PrimitiveRenderer.cpp
)

add_executable(zimovka_tests
    test_vec2.cpp
    test_input.cpp
    test_bullet_system.cpp
    test_player_system.cpp
    test_collision.cpp
    test_debug_accumulator.cpp
    ${ZIMOVKA_TEST_SOURCES}
)

# インクルードパス(親CMakeLists.txtでpkg_check_modulesが実行済みなので変数が使える)
target_include_directories(zimovka_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

# リンク: GTest + SDL2(PrimitiveRenderer.cppとInputSystem.cppが必要)
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
