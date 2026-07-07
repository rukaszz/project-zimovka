# NOTE

雑多なメモ書き．「こうした理由」を記述する．

## 環境や構成，設計の意図のメモ

### 2026/06/23

includeディレクトリの下にzimovkaディレクトリを切って，その下に各種のヘッダファイルを管理するディレクトリを置くようにした．これによって名前空間の衝突等を避けられる．また，公開/非公開を切り分けられたり，インクルードパスのスタイル統一，パッケージ化などのメリットがあるらしい．

### 2026/06/24

#### コーディングスタイルについて

前回の2D横スクロールアクションゲームでは，命名規則が曖昧なまま進めていたので，次の方針で進める．基本的にはGoogleのスタイルガイドを参考にする．

| カテゴリ | 規則 | 例 |
| :---: | :---: | :---: |
| クラス名 | PascalCase | Application, SdlContext |
| 関数・メソッド | PascalCase | Run(), ProcessEvents(), CapFrameRate() |
| ローカル変数 | snake_case | fixed_delta, frame_start_ms |
| privateメンバ変数 | snake_case_ (末尾アンダースコア) | running_, renderer_ |
| static constexpr 定数 | UPPER_SNAKE_CASE | TARGET_FPS, FRAME_DELAY_MS |
| 名前空間 | snake_case | zimovka |
| enum class の値 | PascalCase | Action::MoveLeft, Action::Count |

なお，定数のプレフィックスに"k"をつけるスタイルは使用しない．
定数はアッパースネークケースを採用する．

なお，getter/setterなどのアクセサは判断に迷う．現状は関数名のパスカルケースに従う．
意味を明示するべく，`GetRaw()`などを使用することも考慮したが，現状の`get()→Get()`に修正する方針で進める．

### 2026/06/27

#### 入力ビットマスク

入力システムはビット演算をベースに0/1で進める．
これによってリプレイ時は0/1のストリームを流すことで再現可能．ランダムはseed固定で対応できる想定．

```cpp
static inline constexpr std::uint32_t ActionBit(Action act){
    /**
     * Actionの値を8ビット(符号なし)整数に変換
     * 左シフトで8ビット整数を表現する位置に1を立てる
     * 例：act=MoveLeft→static_cast<std::uint8_t>(act)=2
     * → 1u << 2 となるので左へ2移動するので，8ビット表記すると0b00000100(10進数で4)
     * ※直感的には0b0000 0010(10進数で2)にしたくなるが，0番目の要素(MoveUp)を表現する桁がないので左シフトしている 
     * */
    return 1u << static_cast<std::uint8_t>(act);
}
```

初見ではなかなか理解できないが，1をシフトさせて0b00...00(32ビット)の列に1を立てている．
例えばMoveUpならActionの第一要素なので0番目，であるなので0b00...01になる．
Bombなら7番目なので，7つシフトして，0b00... 0000 1000 0000となり，8番目が1になる．

InputStateはPOD的なデータクラスになっている．PODはCとのABI互換性が保証されたデータ構造で，ビットによって入力を表現する．

#### 代替コンストラクタ

```cpp
/**
 * @brief Bit MaskからInputState型オブジェクトを作成する
 * コンストラクタを介さず外部からBit Maskを渡してInputStateを作れる
 * いわゆる代替コンストラクタ
 * 
 * @param held_bits 
 * @param pressed_bits 
 * @param released_bits 
 * @return InputState 
 */
static InputState FromBits(
    std::uint32_t held_bits, 
    std::uint32_t pressed_bits, 
    std::uint32_t released_bits 
) noexcept 
{
    InputState state;
    state.held_bits_ = held_bits;
    state.pressed_bits_ = pressed_bits;
    state.released_bits_ = released_bits;
    return state;
}
```

FromBitsは静的ファクトリ関数というコンストラクタ的な関数．InputStateインスタンスを返す生成メソッドである．
InputState型変数に各状態の変数を代入して返している．値で返しているが，コンパイラがRVO/NRVOの戻り値最適化をしてくれる．
外部からビットマスクを渡して，直接敵にInputStateを作れるこの仕組みは，コンストラクタ代替(named constructor idiom)という．
オブジェクトを生成する直感的なメソッドであり，初期化を制約を加えつつ実装できる．つまり，コンストラクタの複雑なオーバーロードを減らせる．

#### ブレゼンハムのアルゴリズム

PrimitiveRendererという単純な図形を描画するクラスがある．
このクラスでSDLの線分描画関数を用いて円の描画を実装した．
古典的なアルゴリズムとしてブレゼンハムのアルゴリズム(中点円のアルゴリズム)がある．
本来のブレゼンハムのアルゴリズムは円の1/4や1/8のみピクセル格子上で描画し，のこりは対称性を利用してコピーし円を近似するアルゴリズムである．
ここではもっと単純に，線分を積み重ねて円を表現している．

```cpp
/**
 * @brief 塗りつぶし円描画
 *
 * 中点円アルゴリズム(ブレゼンハムのアルゴリズム)で水平ラインを積み上げて塗りつぶす．
 * r^2 - dy^2 の平方根でその行の横幅を求めSDL_RenderDrawLineで描く
 * 整数の演算のみで線分を描くので高速
 *
 * @param cx 中心X座標(ゲーム座標)
 * @param cy 中心Y座標(ゲーム座標)
 * @param radius 半径(ピクセル)
 * @param color 描画色
 */
void zimovka::PrimitiveRenderer::DrawFilledCircle(float cx, float cy, int radius, SDL_Color color){
    // 色設定
    SetColor(color);
    // intキャスト
    const int icx = static_cast<int>(cx);
    const int icy = static_cast<int>(cy);
    const int r2  = radius * radius;
    // y座標の最小値(-r)からy座標の最大値(r)まで増加させる
    for(int dy = -radius; dy <= radius; ++dy){
        // 0〜r2〜0のように増加するx座標
        const int dx = static_cast<int>(std::sqrt(static_cast<float>(r2 - dy * dy)));
        // 中心(icx, icy)から±dx，±dyの線分を円の下から上へ(積み上げて)塗りつぶすように描いて，円を近似する
        SDL_RenderDrawLine(renderer_, icx - dx, icy + dy, icx + dx, icy + dy);
    }
}
```

例として，$2*2$の円を考えてみる．中心点は任意の(x, y)で良い．ここでは(0, 0)とする．
dx, dyは線分を描画するための変数であり，この変数を用いて下から上へ積み上げるように線分を描画して円を描く．
次のフローで描画が実施される：

1. dy = -2なので，dxは$(2*2)-2*2=0$．よって{x1, y1, x2, y2}: {0, -2, 0, -2}の線分を描く
2. dy=-1なので，dxは$4 - (-1)*(-1)=3$．よって{x1, y1, x2, y2}: {-3, -1, 3, -1}の線分を描く
3. dy=0なので，dxは4．よって{x1, y1, x2, y2}: {-4, 0, 4, 0}の線分を描く
4. dy=1なので，dxは$4 - 1*1=3$．よって{x1, y1, x2, y2}: {-3, 1, 3, 1}の線分を描く
5. dy = -2なので，dxは$(2*2)-2*2=0$．よって{x1, y1, x2, y2}: {0, 2, 0, 2}の線分を描く

これで中心が最長で，上下に行くほど短くなる線分によって円が表現される．

### 2026/06/28

#### Vec2の定義

2次元ベクトルの構造体を定義した．コンストラクタ，汎用メンバ関数(Length()など)に加えて，演算子オーバロードを実装している．
複雑なものではなく，単なるベクトル演算の計算方法に準ずるように演算子の振る舞いを定義しているだけである．

1点注意したいのは，$k * Vec2$という計算である．これは左辺(左オペランド)がVec2ではないため，メンバとして演算子を定義できない．なお，右辺がスカラの場合はメンバに定義できる．
これは定義的にも正しいと言える．理由として，左オペランドがVec2の場合の演算子の定義は，Vec2の振る舞いの定義と言える．しかし右オペランドがVec2の場合は，むしろ外部から見たVec2の操作に近い．ゆえにメンバとして定義できない．

そこで実装ではfriend修飾子を用いている．これによって外部からVec2を直接触ることができている．なお，クラス(struct)外に定義することもできる．その場合は振る舞いをメンバ実装に委譲する形になる：

```cpp
inline Vec2 operator*(float s, const Vec2& v){
    return v*s;
}
```

つまり交換して$k*Vec2$を$Vec2*k$にするのである．
これはメンバ外ではあるが，ADL(Argument Dependent Lookup)というC++名前解決ルールに従って，関数呼び出し時に自動的に関数を探索される．コーニッグ探索(Koening lookup)ともいう．これによって，明示的なusing, 完全修飾名を記述しなくても適切な関数を見つけられる(ことがある)．

通常のC++の名前解決ルールは，まず呼び出し場所のスコープを探して，その外側へと順に範囲を広げる．ADLはその際に，呼び出しの引数型に関連付けされた名前空間(例：型が定義された名前空間，基底型の名前空間など)を探す．

上記の右オペランドの例では，Vec2の定義を探しても計算方法は見つからないが，Vec2に関連する名前空間の検索で，Vec2.hpp内が探索され，構造体外で定義された`operator*`を見つけられる，という動作になる．

非常に参考になるQiitaの記事：

[Vec2クラスを作る](https://qiita.com/Reputeless/items/96226cfe1282a014b147)

※0除算の取り扱いの考慮が必要

### 2026/06/29

PlayerSystemやBulletに，SDL_Colorのためだけに`<SDL2/SDL.h>`をインクルードするのは重たいので，SDL_Colorと同様の定義を持つColor.hppを定義している．

### 2026/06/30

BulletSystemのコンストラクタについて．
コンストラクタではreserve()ではなくresize()を使用している．
これには理由があり，簡単に言うとreserve()ではbegin()とend()が同じ位置を指してしまう．

詳しく言うと，reserve()とresize()は挙動が違う．reserve()で10の容量(capacity)を確保したとき，begin()とend()は同じ位置を指している．つまり，size()==0であり，10個分の容量が確保されているだけ，の状態．

一方resize()は10の容量を確保すると，end()は10(末尾の要素の1つ先)を指す．つまり0〜9の添字が使える配列であり，空の箱を10個用意したような状態となる．そのため，size()==10である．

重要な挙動として，reserve()したあとのpush_backは使用済み要素の次に要素が入る．つまりリスト末尾に追加されていく．resize()は末尾に追加される．すべてのactive==falseの弾を用意するという目的には，resize()が適している．

余談だが，v.reserve(5)をして，v.push_back(10)をした際に，v[3]とかすると空の要素にアクセスしてUB(未定義動作)となる(size()==1である)．
v.resize(5)をして，v[4]をしてもOK．なお，v.resize(n)を実行すると，n回コンストラクタが呼ばれる．

非常に参考にサイト：

[resize vs reserve](https://suzulang.com/c-stdvector-resize%E3%81%A8reserve%E3%81%AE%E9%81%95%E3%81%84-%E5%8A%A0%E7%AD%86%E4%B8%AD%EF%BC%9F/)

### 2026/07/01

#### 円同士の衝突について

円同士の衝突とは2つの円の外縁が交差したら衝突とみなす．そのために必要なのは中心位置である．中心座標と半径を用いて円の交差を判定する．

```text
      円a          円b
<-r1->         <r2>
|-----・-----| |---・---|
       <-----d----->
```

2つの円aとbがちょうど接している状態を考える．これは交差ぎりぎりのエッジケースである．この中心座標aとbの間の距離dを考えると，r1+r2という円aとbの半径の和に等しくなる．
逆に言えば，aとbの距離`d`が`r1+r2`より小さければ，円が交差しているといえる．

円aとbの中心座標の距離dの求め方は2つのベクトルのノルムなので，$\sqrt((x2-x1)^2 + (y2-y1)^2)$という式は2乗すれば平方根が消える．
よって，$d < r1 + r2$という式は，$d^2 \leq (r1 + r2)^2$あるいは$|d|^2 \leq (r1 + r2)^2$と等価であり，平方根ではなく四則演算で交差を高速に判定できる．

なお，不等号の違いもある．$\leq$を用いる場合は円同士の接触も交差と判定する．また，$<$の場合は円同士の接触は衝突とはならない．浮動小数点の誤差や見た目の判定を考えると，等号を含めるのが一般的のようである．また，イプシロンのような小さい誤差吸収用の変数を含める調整も聴きやすい．

#### 高速な衝突判定

ゲームの更新に合わせた衝突判定は，1フレームごとに行われるという特徴がある．
そのため，1フレームで大きく弾が移動した場合，衝突判定をすり抜けるトンネリングが生じる可能性がある．
この事象にしっかりと対応しようと思った場合，円の動く方向を考えたベクトル演算，連続的な未来の座標を用いた演算も必要である．
これは非常に高コストな演算であるが，zimovkaでは不要であると考える．理由としては次の通り．

まずトンネリングが発生する条件：

トンネリングする弾速px/s = (player.radius + bullet.radius) \* 60fps(≒0.016s) = (4 \* 4) \* 60 = 480px/s

つまりプレイヤーの当たり判定4px，弾の当たり判定4pxの場合，1フレームで480px/sも移動する弾速でなければならない．
480 / 60 = 8px/frameの移動速度であり，現状の接触判定ならすり抜けは理論値ならありえる，という感じである．
現在の設計に照らし合わせて，弾速は480px/s以下，安全マージンをとって，300px/sくらいにするのが良さそうである．

またUXの視点で見れば，480px/sの弾速は視認性が悪く，プレイヤーにとって不親切な弾である．
zimovkaのゲームデザインでは，視認性の悪い弾はなるべく使用しない方針であるから，
基本的には上記の単純な円の計算がパフォーマンス面を考慮して良いと考える．

今後実装する場合は，移動を考慮した判定が必要になりコストが高いため，特定の弾のみ別途判定するという局所的な使用に使われる．

参考書籍：実例で学ぶゲーム3D数学(オライリー・ジャパン，オーム社)

### 2026/07/04

DebugOverlayについて．性能試験のために1200発の弾を一気に描画する処理を実装．当初は実装方法が悪く，都度1200発を走査してactiveな弾を補充する方法で実装していた．このときのDebugStatsは次の通り：

| 項目 | 平均値 | 備考 |
| :---: | :---: | :---: |
| frame | 6~7ms | キャラクタ移動時は，一時的に11ms弱くらい |
| update | 4〜5ms | キャラクタ移動時は6〜7msくらい |
| render | 平均1〜2ms | キャラクタ移動時もそこまで増加せず |
| bullets | 1200 / 1200 | 値はほぼ一定 |
| collision | 1200 checks | 値はほぼ一定 |

updateが非常に重たい．全体として，60fpsの維持にはまだ余裕があるが，画像や音声などを取り扱ったりすればギリギリになることが予測される．

原因は複数個あるが，まず第1は弾生成の非効率なループである．メインループ内で更に1200回のループが走っているために，パフォーマンスが悪かった．また，デバッグ情報の描画もSurface/Textureの生成・破棄をループのたびに実装しており，そこもパフォーマンスに影響している．

そこで描画するデバッグ情報を見直し，平均値・最大値を用いて収集する情報を区分した．さらに描画処理も0,25秒に1回として，Surface/Textureの生成処理を減らした．しかしこれによって，デバッグ情報の描画処理がチラつくようになったので，修正が必要である．

現状はDebugビルドのみ計測しているが，余力があればReleaseビルドも計測したい．

チラつく画面から読み取ったデータは次の通り：

| 項目 | 平均値 | 最大値 | 備考 |
| :---: | :---: | :---: |
| frame | 16.64ms | 16.65ms | 1フレームの値 |
| process | 1.5ms | 2.5ms | キャラクタ移動時は+1msくらい(当たり判定の弾の描画などが発生するため？)．ただCPUのジョブの影響が大きそう |
| update | 0.12ms | 0.2ms | キャラクタ移動時もそこまで増加せず |
| render | 1ms | 2.0ms | 弾が最大数のとき，若干増加 |
| steps | 1 | 1 | 1で安定 |
| bullets | 徐々に減少する | 1200 | 値はほぼ一定 |
| collision | 徐々に減少する | 1200 | 値はほぼ一定 |

### 2026/07/06

#### string_viewについて

string_viewはTextTextureで用いている．これはstringの参照を得る機能で，stringのコピーが発生せず早い．
ただし文字列の終端が`\0`であるかは保証されておらず，SDLのAPIへ渡す場合は注意が必要である．

```cpp
// 更新関数 (テキストが変化した場合のみSurface/Textureを再生成)
bool Update(
    SDL_Renderer*    renderer,  // 所有しない
    TTF_Font*        font,      // 所有しない
    std::string_view text,      // 所有権を持たない文字列参照
    zimovka::Color   color      // zimovka::Color (SDL_Colorへの変換は内部で実施)
);
```

難しいのは，この段階では問題にならないことである．`std::format`は終端が`\0`であることが保証されており，
string_viewによる処理の軽量化はむしろ正しいといえる．
しかしTextTextureは今後，スコア表示などに活用することを見越した汎用的になクラスである．
汎用性を重視するなら，string_viewを用いないほうがよく，またSDLのAPIへ渡すことを考えたらstringで渡したほうが安全である．
よって，string_viewを使うことは避ける方針にする．

#### InputSystemと複数キー割当

InputSystemはキーが押された，離れたを1bitのON/OFFで管理している．しかしこの場合，2つのキー割当がなされている操作を実施すると，意図しない動作となる可能性がある．

具体的には，Wキーと上矢印キーの同時押しがわかりやすい．Wキーを押下して，MoveUpが`1`となる．そこで上矢印キーを押すと，bitはそのまま`1`である．ここでWキーを離すと，キーが離れたので-1されbitは0になる．よって，上矢印キーを押しているにもかかわらず，Playerは前進を止めてしまう．

これは0/1管理に対して複数キー割当を考慮していないために発生する．
そこで，押された/離れたをカウントする配列を用意して対応する．

```cpp
/**
 * @brief 同一Actionに複数キーが割り当てられている場合の参照カウント
 *
 * 例: LSHIFT/RSHIFTは両方Action::Slowにマッピングされる
 * hold_count_[Slow] == 2 → どちらかが離れても解除しない
 * hold_count_[Slow] == 0 になって初めてheld/releasedを更新する
 */
std::array<std::uint8_t, ACTION_COUNT> hold_count_{};
```

この配列は複数キーが押されたら+2として，逆に全て離れたら-2することで，押された/離れたを管理する．キー操作時に呼ばれる関数に対して，配列の増減を実施する処理を加える．

```cpp
/**
 * @brief キー押下時のInputState制御関数
 *
 * 同一Actionに複数キーが割り当てられている場合は参照カウントで管理する
 * 例: LSHIFT + RSHIFT → Action::Slow
 *   LShiftを押す  : hold_count[Slow] == 0 → SetHeld(true), SetPressed
 *   RShiftも押す  : hold_count[Slow] == 1 → カウントのみ増加
 *   LShiftを離す  : hold_count[Slow] == 1 → カウント減，まだ1なので held は維持
 *   RShiftを離す  : hold_count[Slow] == 0 → SetHeld(false), SetReleased
 *
 * @param scancode
 * @param repeat  event.key.repeat != 0 なので押され続けている→trueになる
 */
void InputSystem::HandleKeyDown(SDL_Scancode scancode, bool repeat){
    // キーリピートなら無視する※連続発火防止
    if(repeat){
        return;
    }
    // Actionにマッピングする
    zimovka::Action act;
    if(!MapKeyToAction(scancode, act)){
        // マッピングできないキーなら即return
        return;
    }
    // 押されたキーに対応するインデックス取得
    const auto idx = static_cast<std::size_t>(act);
    // 同じActionの最初の押下時(押下カウントが0)のみpressed + heldをセット
    if(hold_counts_[idx] == 0){
        // 前フレームで押されていなければ(heldでなければ)Pressed
        if(!state_.IsHeld(act)){
            state_.SetPressed(act);
        }
        state_.SetHeld(act, true);
    }
    // 押されたカウント+1
    ++hold_counts_[idx];
}

/**
 * @brief キーが離れたときのInputState制御関数
 *
 * hold_count_が0になって初めてreleased + heldクリアをセットする
 *
 * @param scancode
 */
void InputSystem::HandleKeyUp(SDL_Scancode scancode){
    // Actionにマッピング
    zimovka::Action act;
    if(!MapKeyToAction(scancode, act)){
        // マッピングできないキーなら即return
        return;
    }
    // 押されたキーに対応するインデックス取得
    const auto idx = static_cast<std::size_t>(act);
    // 押されたカウントが1以上なら-1
    if(hold_counts_[idx] > 0){
        --hold_counts_[idx];
    }
    // 同じActionに対応する全キーが離れた時のみheld解除 + released
    // ex) wキー+↑キーでwキーを離しても上方向へ移動させるため
    if(hold_counts_[idx] == 0){
        state_.SetHeld(act, false);
        state_.SetReleased(act);
    }
}
```

これによって既存の1bitのマスクによる設計はそのままで，内部カウントを追加して対応できた．こうすると，リプレイを0/1のビットマスクで対応する当初の想定が崩れない．

余談だが，ウィンドウのフォーカスが離れた際にキー押下状態のまま担ってしまうという状況が発生する可能性がある．
そのため，SDLの`SDL_WINDOWEVENT_FOCUS_LOST`を用いて，ウィンドウが非活性になったらキーの状態をリセットする処理を加えている．

```cpp
/**
 * @brief キー押下イベント処理(ProcessEvents()で呼ばれる)
 *
 * @param event
 */
void InputSystem::HandleEvent(const SDL_Event& event){
    // キー押す/離すの操作に応じて関数を呼ぶ
    if(event.type == SDL_KEYDOWN){
        HandleKeyDown(event.key.keysym.scancode, event.key.repeat != 0);
    } else if(event.type == SDL_KEYUP){
        HandleKeyUp(event.key.keysym.scancode);
    } else if(event.type == SDL_WINDOWEVENT &&
              event.window.event == SDL_WINDOWEVENT_FOCUS_LOST){
        // フォーカス喪失時はSDL_KEYUPが届かないため手動で全状態をリセット
        // 参照カウントの整合性を保つためhold_counts_もリセット
        state_.ClearAll();
        hold_counts_.fill(0);
    }
}
```
