# NOTE

雑多なメモ書き．「こうした理由」を記述する．

## 環境や構成について

### 2026/06/23

includeディレクトリの下にzimovkaディレクトリを切って，その下に各種のヘッダファイルを管理するディレクトリを置くようにした．これによって名前空間の衝突等を避けられる．また，公開/非公開を切り分けられたり，インクルードパスのスタイル統一，パッケージ化などのメリットがあるらしい．

### 2026/06/24

コーディングスタイルについて．

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
