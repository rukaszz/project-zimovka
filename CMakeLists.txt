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

# ソースファイル
# ※クラスを追加したらここにも追記する
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/app/Application.cpp
)

# インクルードパス
# include/zimovka/... に置いたヘッダが "zimovka/..." でincludeできる
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
