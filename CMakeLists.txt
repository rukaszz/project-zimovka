# CMakeの最低バージョン要求(これ未満はエラーになる)
cmake_minimum_required(VERSION 3.16)
# プロジェクト名の定義と，言語はC++を使うという宣言
project(zimovka LANGUAGES CXX)

# 指定するC++のバージョン(C++17でのビルド等を防ぐ)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# デフォルトビルドタイプをDebugに設定(未指定時)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type: Debug or Release" FORCE)
endif()

# 実行ファイルの出力先(binに出力)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# pkg-config を利用してSDL2系ライブラリを検索(Linux)
# TODO: Windows用の対応も必要
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2       REQUIRED sdl2)
pkg_check_modules(SDL2IMAGE  REQUIRED SDL2_image)
pkg_check_modules(SDL2TTF    REQUIRED SDL2_ttf)
pkg_check_modules(SDL2MIXER  REQUIRED SDL2_mixer)

# ──────────────────────────────────────────────────────
# ソースファイルの分類
# ──────────────────────────────────────────────────────

# ── ゲームシステム本体(CORE) ──────────────────────────
# SDL不要・テスト可能なゲームシステム群
# 新しいシステムクラスを追加したらここに1行追記するだけでOK
# unit tests/integration tests/zimovka本体のそれぞれに適用される
set(ZIMOVKA_CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/DeterministicRng.cpp
    ${CMAKE_SOURCE_DIR}/src/input/InputState.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/PrimitiveRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/bomb/PlayerBombSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/bullet/BulletSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/pattern/PatternSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/player/PlayerSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/player/PlayerWeaponSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/enemy/EnemySystem.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/collision/CollisionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/replay/RunRecorder.cpp
)

# ── エンジン層(ENGINE) ────────────────────────────────
# UpdatePipeline + リプレイ系
# unit testでは不要/複合試験・本体で使用する
set(ZIMOVKA_ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/engine/update/UpdatePipeline.cpp
)

# ── 本体専用ソース(SDL依存・テスト対象外) ──────────────
# InputSystem(SDL events), Platform/Rendering/App/Debug層
# 原則zimovka本体で使う
set(ZIMOVKA_APP_SOURCES
    src/input/InputSystem.cpp
    src/platform/SdlContext.cpp
    src/platform/Window.cpp
    src/rendering/Renderer.cpp
    src/rendering/TextTexture.cpp
    src/app/Application.cpp
    src/debug/DebugOverlay.cpp
)

# ── 本体ビルド ────────────────────────────────────────
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${ZIMOVKA_CORE_SOURCES}
    ${ZIMOVKA_ENGINE_SOURCES}
    ${ZIMOVKA_APP_SOURCES}
)

# インクルードパス
# include/zimovka/... に置いたヘッダが"zimovka/..."でincludeできる
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${SDL2IMAGE_INCLUDE_DIRS}
    ${SDL2TTF_INCLUDE_DIRS}
    ${SDL2MIXER_INCLUDE_DIRS}
)

# リンクするライブラリ
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2IMAGE_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
    ${SDL2MIXER_LIBRARIES}
)

# コンパイルオプション
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic            # 警告レベルは最大
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>   # デバッグ時は最適化しない
    $<$<CONFIG:Release>:-O2 -DNDEBUG>   # リリース用はO2の最適化を適用
)

# ──────────────────────────────────────────────────────
# テスト(GoogleTest)
# BUILD_TESTING=OFFでスキップできる
# BUILD_INTEGRATION_TESTING=ONで複合試験を追加ビルド
# ──────────────────────────────────────────────────────
option(BUILD_TESTING             "Build unit tests"                    ON)
option(BUILD_INTEGRATION_TESTING "Build integration/performance tests" OFF) # デフォルトOFF

# GoogleTestをFetchContentで取得(単体テスト・複合試験共通)
if(BUILD_TESTING OR BUILD_INTEGRATION_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    # Windows向け: gtestが使用するランタイムDLLのリンク設定を統一する
    # 既定値は上書きする
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
if(BUILD_INTEGRATION_TESTING)
    add_subdirectory(integration_tests)
endif()

# ── ビルドコマンド ────────────────────────────────────────
## 設定
### 初回 or CMakeLists.txt変更時
# cmake -B build
### integration testsを含める場合
# cmake -B build -DBUILD_INTEGRATION_TESTING=ON

## ビルド
### 本体
# cmake --build build --target zimovka

### unit tests
# cmake --build build --target zimovka_tests

### integration tests
# cmake --build build --target zimovka_integration_tests

### 全ターゲット
# cmake --build build

# デフォルトはDebugビルドなので， -G -O0
# リリース(-g -O2)にしたい場合は
# cmake -B build -DCMAKE_BUILD_TYPE=Release

## Ninjaを用いて高速でビルドする場合
### 設定（-G Ninja を追加）
# cmake -G Ninja -B build

### ビルドはそのまま（cmake --build は自動でNinjaを使う）
# cmake --build build --target zimovka_tests

# -G Ninjaオプションを使う場合はbuildディレクトリがNinjaで設定される
# Make⇔Ninjaの変更は，buildディレクトリの削除→再設定が必要
