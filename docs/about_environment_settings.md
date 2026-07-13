# Zimovkaの環境設定について

## 概要

本プロジェクトZimovkaのプログラミング環境を整備したため，その各種の設定について，備忘録として残す．
プロジェクト特有の環境依存的な箇所が存在する可能性はあるが，全体的にはプロジェクト開始の際に広く流用できる内容である(はず)．

なお，ZimovkaはVisial Studio Code(以下vscode)を用いて開発しているので，CMakeLists.txtとvscode独自の設定を記している．

## CMakeLists.txt

### CMakeLists 概要

言わずとしれたC++のビルドを自動化するためのシステムCMakeとその設定ファイル．どのファイルが，どのオプションで，どのライブラリとリンクさせてビルドするか，を定義する．

OSや開発環境関係の専用設定に影響されがちなC/C++のコンパイル作業を，CMakeLists.txtに記述することで環境的な差異を吸収してビルドを行ってくれる．クロスプラットホーム対応かつ，外部ライブラリなどを自動で探して調整してくれる．

ZimovkaのCMakeLists.txt：

```cmake
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

# pkg-config を利用してSDL2系ライブラリを検索
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

```

### CMakeLits 各種記述の解説

プロジェクトの定義などを冒頭で実施する．
例えば，Cやアセンブリ言語を本プロジェクトで使用するのであれば，`LANGUAGES CXX`がそれを防ぐ．

プロジェクト名の定義なども冒頭で定義する．これによって，以降は`${PROJET_NAME}`という変数を用いてプロジェクト名を参照できるようになる．

```cmake
# CMakeの最低バージョン要求(これ未満はエラーになる)
cmake_minimum_required(VERSION 3.16)
# プロジェクト名の定義と，言語はC++を使うという宣言
project(zimovka LANGUAGES CXX)
```

次に本プロジェクトではC++20を使用するため，それらを宣言する．C++23については，本プロジェクト開始時点では主要コンパイラが部分実装であることを考慮して不採用とした．

`set(CMAKE_CXX_STANDARD_REQUIRED ON)`はC++20に対応していない古いコンパイラなどで，C++20を使わずにコンパイルしようとすることを防ぐ定義である．

```cmake
# 指定するC++のバージョン(C++17でのビルド等を防ぐ)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

次にビルド時のオプションである．ゲーム制作では基本的にデバッグのためにビルドすることが多いため，デフォルトでのビルドはデバッグモードになるようにしている．

例えば，`cmake -DCMAKE_BUILD_TYPE=Debug`のように明示的に指定しなくても，単に`cmake -S . -B build`を実行するとデバッグ状態でのビルドになる．

```cmake
# デフォルトビルドタイプをDebugに設定(未指定時)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type: Debug or Release" FORCE)
endif()
```

実行ファイルはbinディレクトリへ出力したいので，それも`set()`を用いて定義する．なお，`${CMAKE_SOURCE_DIR}`はCMakeLists.txtが存在するディレクトリ(つまりプロジェクトのルート)を指す変数である．

```cmake
# 実行ファイルの出力先(binに出力)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
```

SDK等の各種の外部ライブラリのインクルードパスやリンクフラグを取得するために，`pkg-config`を使用する．`pkg-config`はLinux環境でのみ使用できる．各種の`REQUIRED`は，発見できなければエラーとするための指定である．

```cmake
# pkg-config を利用してSDL2系ライブラリを検索
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2       REQUIRED sdl2)
pkg_check_modules(SDL2IMAGE  REQUIRED SDL2_image)
pkg_check_modules(SDL2TTF    REQUIRED SDL2_ttf)
pkg_check_modules(SDL2MIXER  REQUIRED SDL2_mixer)
```

ビルド対象のファイルの指定が続く．新しい`.cpp`ファイルはここに追記していく．なお，ヘッダファイルは`.cpp`ファイル内のインクルードの記述と後述のディレクトリの指定によって，CMakeが良い感じに解決してくれるので記述は不要となる．

```cmake
# ソースファイル
# ※クラスを追加したらここにも追記する
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/app/Application.cpp
)
```

ビルドに関するディレクトリ等の指定．ここでファイル検索時のディレクトリを指定できる．今回はヘッダファイルは`include`ディレクトリへ配置するため，ここに記述している．

PRIVATEの記述は，インクルードパスのスコープを指定しているそうである．例えば外部へ公開するライブラリを作るのであれば，PUBLICを指定することで，自身/依存先の双方でここで指定したディレクトリが使われます．INTERFACEを使うと，このプロジェクトでは参照しないけど，依存先のプロジェクトは参照する，というケースで依存先のビルドで使われます．基本的に，一プロジェクトで完結するのであれば，PRIVATE指定でOKのようです．

```cmake
# インクルードパス
# include/zimovka/... に置いたヘッダが "zimovka/..." でincludeできる
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${SDL2IMAGE_INCLUDE_DIRS}
    ${SDL2TTF_INCLUDE_DIRS}
    ${SDL2MIXER_INCLUDE_DIRS}
)
```

実行ファイルにリンクされるライブラリを次に指定する．今回のプロジェクトでは主にSDL2関係のライブラリとなる．
`${SDL2_LIBRARIES}`には，pkg_check_modulesが取得したコンパイル時に指定する`-lSDL2`などのフラグを持っているイメージである．

```cmake
# リンクするライブラリ
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2IMAGE_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
    ${SDL2MIXER_LIBRARIES}
)
```

最後にコンパイル時のオプションを指定している．おおよそ基本的な設定のみである．
重要なのは`generator expressions`と夜bれるCMake独自の式である．
`$<>`で囲んで式を宣言し，条件式が登場したら再度`$<>`を用いる．

`$<$<CONFIG:Debug>:-g -O0 -DDEBUG>`これはオプションで`-debug`が指定されたら`-g -O0 -DDEBUG`このオプションでコンパイルする，ということである．なお，`-O0`はオー(O)とゼロ(0)である．

参考：[generator expressions](https://qiita.com/mrk_21/items/57075ce36f49ce0aacf4)

```cmake
# コンパイルオプション
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic            # 警告レベルは最大
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>   # デバッグ時は最適化しない
    $<$<CONFIG:Release>:-O2 -DNDEBUG>   # リリース用はO2の最適化を適用
)
```

それぞれのオプションの意味は次の通り：

- `-Wall`
  - 全ての警告(warning)を表示する
- `-Wextra`
  - wallよりも厳密に，コーディングスタイルや論理的な不整合を見つける(未使用な引数など)
- `-Wpedantic`
  - ISO C++に準拠していない独自の拡張(コンパイラ特有のもの)などを検知する(環境依存を防ぐため)
- `-O`
  - 最適化オプション．後ろに0から3まで指定できる※基本的には2までっぽい
  - `0`なら最適化はほぼされない．コードがそのまま機械語に展開されるので，デバッグがやりやすい
  - `2`は実行速度の最大化を目的に強力な最適化を実施する．リリースビルドで推奨される

### GoogleTest用のCMakeLists.txtの記述

#### GoogleTestについて

Googleが開発したC++向けの単体テスト自動化フレームワーク．OSSでありクロスプラットホーム対応，アサーションが豊富でテストから実行までが容易，モックもある，といたれりつくせりなフレームワークである．

Project Zimovkaでは，単体テストはGoogleTestを用いて実施する．

#### CMakeListの内容

```cmake
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
    ${CMAKE_SOURCE_DIR}/src/systems/collision/CollisionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/PrimitiveRenderer.cpp
)

# ビルドする実行ファイル設定
add_executable(zimovka_tests
    test_Vec2.cpp
    test_Input.cpp
    test_BulletSystem.cpp
    test_PlayerSystem.cpp
    test_Collision.cpp
    test_DebugAccumulator.cpp
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

```

#### CMakeListsの内容について

```cmake
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
```

- `include(FetchContent)`
  - CMake 3.11以降で使えるモジュール
  - 外部依存するライブラリを，「CMakeが自動でダウンロードしてビルドする」仕組み
  - FetchContentを用いると，外部コンテンツの依存をCMakeList内に留められる
- `FetchContent_Declare()`
  - 外部コンテンツ(GoogleTest)を宣言する
    - この時点では有効化されていない
  - 宣言の順に次の意味を持つ
    - googletest：この依存関係の名前
    - GIT_REPOSITORY ...：どのGitリポジトリから取得するか
    - GIT_TAG：どのコミット/タグを使用するか
- `FetchContent_MakeAvailable()`
  - これで宣言した`googletest`を与えると
    - `git clone`される
    - CMakeがライブラリをconfigure & buildする
- `set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)`
  - Windows + MSVC環境で，GoogleTestが共有ランライムライブラリを使用するように強制できる
  - `gtest_force_shared_crt`
    - GoogleTestがランタイムライブラリ(CRT)に共有ランタイム(DLL)を使うことを指定するオプション
  - `ON CACHE BOOL`
    - CMake configureで上記のオプションを指定する必要がある
    - set()ではconfigureまでセットされないので，CACHEを指定してキャッシュに書き込んでいる
  - `FORCE`
    - キャッシュへの書き込みで上書きさせる
  - MSVCのランタイムライブラリについて
    - `/MD`：共有ランタイム(DLL)(リリース用)
    - `/MDd`：共有ランタイム(デバッグ用)
    - `/MT`：静的ランタイム(lib)(リリース用)
    - `/MTd`：静的ランタイム(デバッグ用)
    - Windowsの標準ライブラリはDLLで提供されており，共有ランタイム(DLL)が事実上標準となっている
    - 静的リンクは実行ファイル肥大化などを招き，また/MDと併用すると未定義動作になる→共有ランタイムをGoogleTestでも使うようにしている

```cmake
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
    ${CMAKE_SOURCE_DIR}/src/systems/collision/CollisionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/rendering/PrimitiveRenderer.cpp
)

# ビルドする実行ファイル設定
add_executable(zimovka_tests
    test_Vec2.cpp
    test_Input.cpp
    test_BulletSystem.cpp
    test_PlayerSystem.cpp
    test_Collision.cpp
    test_DebugAccumulator.cpp
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

```

重複する部分が多いため全体的な流れのみ説明する．
流れとしては，set→add_executable→target_include_directories→target_link_librariesとなっている．

- `set`
  - ビルドに必要な実装(cppファイル)を宣言
- `add_executable`
  - テストコードと，テストする実装ファイルを宣言
- `target_include directories`
  - テストコードが必要とするヘッダの探索パスを追加
  - 親となるプロジェクトのCMakeListsも参照される
- `target_link_libraries`
  - テストの実行ファイルに`gtest/gtest_main`をリンクする

```cmake
# CTestへ自動登録
include(GoogleTest)
gtest_discover_tests(zimovka_tests)
```

- `include(GoogleTest)`
  - GoogleTest.cmakeというモジュールを読み込む命令
  - CTestというテストランナーと関連付ける
- `gtest_discover_tests(zimovka_tests)`
  - テストの実行ファイル`zimovka_tests`を実行し，内部の`TEST()`を自動検出し，CTestに登録させる
  - 実行時にGoogleTestがCTestへテスト一覧を渡している
  - TEST()とCTestの結びつきをしてくれる

#### GoogleTestの基礎

テストしたい実装のヘッダを読み込んで，`TEST()`にテスト内容を記述していく：

```cpp
#include <gtest/gtest.h>

#include "sample.hpp"

TEST(IsEvenTest, Negative) {
    EXPECT_FALSE(IsEven(-1));
    EXPECT_TRUE(IsEven(-2));
}

TEST(IsEvenTest, Zero) {
    EXPECT_TRUE(IsEven(0));
}

TEST(IsEvenTest, Positive) {
    EXPECT_FALSE(IsEven(1));
    EXPECT_TRUE(IsEven(2));
}

```

実行結果の例：

```bash
# 実行結果
Running main() from /usr/local/src/googletest-release-1.8.1/googletest/src/gtest_main.cc
[==========] Running 3 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 3 tests from IsEvenTest
[ RUN      ] IsEvenTest.Negative
[       OK ] IsEvenTest.Negative (0 ms)
[ RUN      ] IsEvenTest.Zero
[       OK ] IsEvenTest.Zero (0 ms)
[ RUN      ] IsEvenTest.Positive
[       OK ] IsEvenTest.Positive (0 ms)
[----------] 3 tests from IsEvenTest (0 ms total)

[----------] Global test environment tear-down
[==========] 3 tests from 1 test case ran. (0 ms total)
[  PASSED  ] 3 tests.
```

テストしたテストでパスしたものは`OK`，失敗したものは`FAILED`となる．成功したテストの合計は`PASSED`に出る．

テスト関数は`TEST()`を使い，第1引数にテストケース名、第2引数にテスト名を記述する：

```cpp
// テストケース名とテスト名には _ を含んではいけない
TEST(/* テストケース名(大項目)*/, /* テスト名(小項目) */) {
  // テスト関数内は、通常通り C++ のコードを記述可能
}
```

アサーションはいくつも用意されているため，代表的なものを列挙する：

```cpp
// true/falseのアサーション
EXPECT_TRUE(condition);     // condition が true  か
EXPECT_FALSE(condition);    // condition が false か

// 2つの値を比較するアサーション
EXPECT_EQ(expected, actual);  // expected == actual か
EXPECT_NE(expected, actual);  // expected != actual か
EXPECT_LT(expected, actual);  // expected <  actual か
EXPECT_LE(expected, actual);  // expected <= actual か
EXPECT_GT(expected, actual);  // expected >  actual か
EXPECT_GE(expected, actual);  // expected >= actual か
```

これらのアサーションを利用する場合は，期待結果 (expected)→テスト対象 (actual)の順で記述する

`EXPECT_`で始まるアサーションの他に，`ASSERT_`で始まるアサーションがある．`EXPECT_`の場合は，テストに失敗してもテスト関数がそのまま続行されるが，`ASSERT_`の場合は，テストに失敗するとその時点でテストを中断してテスト関数を抜ける．例としてはゼロ除算やテストの前提となる直前の処理がうまく成功したかどうかなど，テストの本筋とは関係ない前提条件などで用いる．

参考：[Google Test の使い方](https://rinatz.github.io/cpp-book/test-how-to-gtest/)

## vscodeの設定

`.vscode`に格納する，プロジェクトの設定について解説する．
なお拡張機能として，これを使用するためには`C/C++ extensions`や`ms-vscode.cmake-tools`が必要である．

実行方法：

`Ctrl + Shift + D`で"Run and debug"を表示．
`Debug: zimovka`を選んだ状態で実行．
初回は多少時間がかかるが実行できる．

### c_cpp_properties.json

#### c_cpp_properties.json 概要

vscodeでC/C++で開発をする際に使用する設定ファイル．インテリセンス(IntelliSense)という，補完・定義へのジャンプ・エラー表示の設定を記述する．vscodeというエディタの設定で，解析等のエンジンの設定となる．

Zimovkaのc_cpp_properties.jsonの内容：

```json
{
    "version": 4,
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/include/**",
                "/usr/include/SDL2",
                "/usr/local/include/SDL2"
            ],
            "defines": ["DEBUG"],
            "compilerPath": "/usr/bin/g++",
            "cppStandard": "c++20",
            "intelliSenseMode": "linux-gcc-x64",
            "configurationProvider": "ms-vscode.cmake-tools"
        }
    ]
}
```

#### c_cpp_properties.json 各記述の解説

参考：[c_cpp_properties](https://code.visualstudio.com/docs/cpp/customize-cpp-settings)

- `version`：このファイルそのもののバージョンを認識するために使用されるようで，基本的には編集は非推奨．拡張機能との設定の確認で使われる様子
- `env`：にユーザ定義環境変数を書ける(versionなどと同階層)
- `configurations`：名の通り，インテリセンスのエンジンにプロジェクトの情報は構成などを教えるための記述を行う場所である．ここに諸々の設定を記述していく
  - `name`：この構成を識別するための名前だが，`Linux, Mac, Win32`などはプラットフォームで選択される特別な識別子である
  - `includePath`：ソースファイルによってインクルードされるディレクトリ．この書き方(include/**)なら，`include`ディレクトリ配下のファイルが再帰的に検索される
  - `defines`：インテリセンスエンジンが使用するプリプロセッサ定義のリストを指定する
  - `compilerPath`：プロジェクトをビルドするためのコンパイラへの絶対パスを指定する．CMakeLists.txtの`target_include_directories`に一致しないと，赤波線による警告が出る．インテリセンスエンジンの精度向上に寄与するらしい
  - `cppStandard`：インテリセンスで使用するC++の標準バージョン(C++20の構文を認識させるのに必要)
  - `intelliSenseMode`：gccやclangなどのアーキテクチャ特有の差異(バリアント)に対応するためのモード(GCC on Linux x64組み込みマクロ定数群が正しく解析できるらしい)．プラットフォーム毎に特有のデフォルト設定がある．Linuxは`linux-gcc-x64`
  - `configurationProvider`：インテリセンスへ構成情報等を提供する拡張機能のID．例えば`CMake Tools`拡張機能を使っていれば，`ms-vscode.cmake-tools`と記述することで，`CMake Tools`の拡張機能の構成情報を参照でき，`includePath`の情報より正確な情報が得られるらしい

### launch.json

#### launch.json 概要

デバッガの起動設定などを記述する．vscodeでGUIデバッグをする場合に設定が必要となる．具体的には，`F5`押下時の動作定義である．

Zimavkaのlaunch.jsonの内容：

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug: zimovka (GDB)",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/bin/zimovka",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "setupCommands": [
                {
                    "description": "GDBのpretty-printingを有効化",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "STLコンテナの中身を読みやすく表示",
                    "text": "set print elements 50",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "CMake: Build (Debug)"
        }
    ]
}
```

#### launch.json の各記述の解説

参考：[GUIデバッグ](https://rinatz.github.io/cpp-book/debug-vscode/)

- `version`：拡張機能やvscodeが構造の解釈や処理で必要とする基本的な情報の定義．基本的に編集不要
- `configurations`：gdbの各種設定を記述する
  - `name`：このlaunch.json固有の識別子
  - `type: "cppdbg"`：C/C++拡張機能のデバッグ機能を使うので，`cppdbg`を指定する(標準のデバッガ)
  - `request: "launch"`：launchはプログラムを起動してデバッグする方式を指定している
  - `program`：デバッグ対象の実行ファイルパス．CMakeLists.txtでの出力先と一致している
  - `args`：CMake側で指定しているので不要
  - `stopAtEntry`：プログラム開始時にプログラムを一時停止(break)させる
  - `cwd`：プログラムの作業ディレクトリ．assetsなどのファイルを読み込む際の基準であり，プログラム内での相対パスの記述と一致させないといけない※`assert/...`のように書くなら基本的にOK
  - `environment`：デバッグ時の環境変数を記述する
  - `externalConsole`：標準入力などを受け付ける用のターミナルを起動させる
  - `MIMode`：デバッグ時に裏で動作させるデバッガの指定．今回ならgdb
  - `miDebuggerPath`：名の通りデバッガの所在地を指定する
    - なおこれらはMI(Machine Interface)モードで起動するので，vscodeがgdbとやり取りできる
  - `setupCommands`：gdb起動時に実行するコマンドを指定する
    - `-enable-pretty-printing`：`std::vector`などの中身がvscodeウォッチウィンドウで読みやすくなる(生アドレスが並ぶのを抑制する)
    - `set print elements 50`：コンテナの表示要素数を50個までに制限する(これによって，巨大な配列の表示で固まらないようにする)
  - `preLaunchTask`：デバッガ起動前に実施するタスクを指定する．これはtask.jsonと結びついている

### tasks.json

#### tasks.json 概要

vscodeのタスク機能を設定するファイルである．イメージとしては，ターミナルで実行などで打ち込むコマンドをvscodeから実行できるようにするイメージ．

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake: Configure (Debug)",
            "type": "shell",
            "command": "cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug",
            "group": "build",
            "presentation": {
                "reveal": "always",
                "panel": "shared"
            },
            "problemMatcher": []
        },
        {
            "label": "CMake: Build (Debug)",
            "type": "shell",
            "command": "cmake --build build -- -j$(nproc)",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "dependsOn": "CMake: Configure (Debug)",
            "presentation": {
                "reveal": "always",
                "panel": "shared"
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "CMake: Configure (Release)",
            "type": "shell",
            "command": "cmake -S . -B build_release -DCMAKE_BUILD_TYPE=Release",
            "group": "build",
            "presentation": {
                "reveal": "always",
                "panel": "shared"
            },
            "problemMatcher": []
        },
        {
            "label": "CMake: Build (Release)",
            "type": "shell",
            "command": "cmake --build build_release -- -j$(nproc)",
            "group": "build",
            "dependsOn": "CMake: Configure (Release)",
            "presentation": {
                "reveal": "always",
                "panel": "shared"
            },
            "problemMatcher": ["$gcc"]
        }
    ]
}
```

#### tasks.json 各記述の解説

参考：[デバッグ方法](https://kinoshita-hidetoshi.github.io/Programing-Items/Microsoft/vscode/vscode_json.html)

- `version`：拡張機能やvscodeが構造の解釈や処理で必要とする基本的な情報の定義．基本的に編集不要
- `tasks`：項目は基本的に次の並びで記述される
  - `label`：タスク名．今回の場合はlaunch.jsonが参照する
  - `type`：タスクの実行方法やどう扱うか，を定義する．`shell`を指定すると，システムのシェル経由のタスクとなる．スクリプトを実行するなら`process`を指定する
  - `command`：実行するコマンド．
    - `"cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug"`：buildディレクトリでビルドを実行する．コンパイル時の設定の指定は`Debug`
    - `"cmake --build build -- -j$(nproc)"`：ディレクトリでビルドを実行する際に，CPUコア数に応じて，並列でコンパイルを実行してビルドを高速化する
  - `group`：ビルドタスクの登録．`"isDefault": true`を指定しているタスクは，`Ctrl + Shift + B`によって実行できる
  - `dependsOn`：このタスクが依存している先．ここで指定したタスクの実行後にこのタスクは実行される
  - `presantation`：タスク実行時のターミナルの表示の振る舞いに関する設定
    - `"reveal": "always"`：タスク実行と同時にターミナルをアクティブにする
    - `"panel": "shared"`：出力を共有のターミナルへ出す(複数個ターミナルが出ないように)
  - `"problemMatcher": ["$gcc"]`：ビルドエラーの出力を解析するパターンの指定．`$gcc`はGCC形式のエラーを認識し，どこでエラーとなったかを表示できる
