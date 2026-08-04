# 複合試験(フェーズゼロ)

## 概要

project_zimovka.mdにある通り，第零段階の技術検証として，次の基準を設けている：

> ゲームの全体的な設計・技術的な検証を行う：
> 
> - 自機描画・移動
> - 8方向移動・低速移動
> - 自機弾・敵弾の実装・描画
>   - 1000発の弾表示(性能テスト)
> - 当たり判定の実装
>   - 円の当たり判定
>   - 円の近似(矩形)の当たり判定
> - デバッグ機能の実装
> - 入力・ゲーム環境の記録処理
>   - あくまで記録
> 
> この段階では，次のステップへ進むための明確な条件を設ける：
> 
> - 弾が1000発描画されても60FPSで動く
>   - 具体例：Ubuntu 24.04 LTS の開発環境で，オブジェクトを表示・更新して平均60FPSを維持
>     - 敵弾1000発のactive
>     - 自機弾100発
>     - 敵10体
> - デバッグ情報を取得して次の情報を確認できる
>   - frame time
>   - update time
>   - render time
>   - active bullet count
> - 自機が動ける・避けられる
>   - 8方向の移動
>   - 低速移動
> - ゲーム中の各情報が収集できる
>   - 活性状態の弾
>   - 乱数seed固定
>   - 入力の記録
> - 各種オブジェクトの生成・更新・描画が安定している
> 
> ※安定とは，1フレームの平均デルタ時間が1/60秒を超えてしまい，更新待ちが累積して**update処理が遅延するようなことが無い**ことを言う

第零段階完了のために複合試験で性能試験を実施する．
※技術的な基礎の実装であることから，特に詳細に実施する必要があると考えているからである．

## 複合試験シナリオ

### 環境・構成

性能試験用のビルドPERF_TEST(Performance test)を用意する．

配置は次のようにする：

- Player
  - (x, y)=(900, 600)
- Enemy Bullet
  - 1200個
  - velocity=0
    - 常にactiveにするため
- Enemy
  - 10体
  - y=100
  - x=0, 50, 100, ...
- Player Bullet
  - 100個
    - 今後これほど表示する可能性は低いが，性能試験のために実施
  - Enemyには接触しない
    - x=900, y=500付近に重なって良いので配置
  - velocity=0

### 定常負荷試験(Steady)

600Tickを想定し，更新を複数回実施しても出力が安定しているかを見る．
特に確認するべきは次の内容であり，これらが安定していることを保証する目的がある：

- active数がぶれない
- オブジェクトが意図せず消えない(inactiveにならない)
- 計測値が正確
- 冪等性がある(同じ状態，同じ結果になる)
- active_count_が正確

検証用のソースは次のイメージ：

```cpp
for(std::size_t tick = 0; tick < 600; ++tick){
    collision_system.InitializeStatsAtBeginTick();
    // 更新
    enemy_bullets.Update(
        FIXED_DELTA,
        WORLD_WIDTH,
        WORLD_HEIGHT
    );
    player_bullets.Update(
        FIXED_DELTA,
        WORLD_WIDTH,
        WORLD_HEIGHT
    );
    enemies.Update(
        FIXED_DELTA,
        WORLD_WIDTH,
        WORLD_HEIGHT
    );
    // 衝突判定(自機vs敵弾)
    EXPECT_FALSE(
        collision_system
            .CheckPlayerHitByBullets(
                player,
                enemy_bullets
            )
    );
    // 衝突判定(自機弾vs敵)
    const EnemyHitEvents hits =
        collision_system
            .ResolvePlayerBulletsVsEnemies(
                player_bullets,
                enemies
            );
    // ヒットカウント(定常試験なのでゼロ)
    ASSERT_EQ(hits.hit_count, 0u);
    ASSERT_EQ(hits.kill_count, 0u);
    // active数計測
    ASSERT_EQ(enemy_bullets.CountActive(), 1200u);
    ASSERT_EQ(player_bullets.CountActive(), 100u);
    ASSERT_EQ(enemies.CountActive(), 10u);
    // 自機と敵弾の衝突判定走査
    ASSERT_EQ(
        collision_system.GetStats()
            .player_vs_enemy_bullet_checks,
        1200u
    );
    // 自機弾と敵の衝突判定走査
    ASSERT_EQ(
        collision_system.GetStats()
            .player_bullet_vs_enemy_checks,
        1000u
    );
}
```

### 衝突・負荷試験(Collision Churn)

定常負荷試験とは異なり，状態遷移が多い試験となる．ここでの状態遷移とは，次のようなパターンが考えられる：

- 自機弾と敵の衝突解決
  - 敵10体をHP=1で配置
  - 自機弾を10発配置
- 衝突解決  
  - 敵がinactiveになる
  - 自機弾がinactiveになる
- 空きスロットと再Spawn
  - 同じ空きスロットに再Spawn可能
  - 敵が再出現する
- 1000サイクルSpawnとinactiveによる消滅を繰り返す

ここでもゲームシステムが安定していることを確認する必要がある：

- active数がぶれない(active_count_が正確)
- オブジェクトが意図せず消えない(inactiveにならない)
- 計測値が正確
- 同一初期状態と同一入力から、同一結果が得られる
- 空きスロットが再利用できる
- メモリの連続性
- hit/killの一致

検証用ソースのイメージ：

```cpp
for (std::size_t cycle = 0; cycle < 1000; ++cycle){
    // 敵/自機弾出現処理
    for (std::size_t i = 0; i < 10; ++i){
        const Vec2 position{
            50.0f + static_cast<float>(i) * 80.0f,
            100.0f
        };

        ASSERT_TRUE(
            enemies.Spawn(
                MakeEnemyParams(position, 1)
            )
        );

        ASSERT_TRUE(
            player_bullets.Spawn(
                position,
                {0.0f, 0.0f},
                3.0f
            )
        );
    }

    collision_system.BeginTick();
    // 自機弾vs敵
    const EnemyHitEvents hits =
        collision_system
            .ResolvePlayerBulletsVsEnemies(
                player_bullets,
                enemies
            );
    // 各種計測
    ASSERT_EQ(hits.hit_count, 10u);
    ASSERT_EQ(hits.kill_count, 10u);
    ASSERT_EQ(enemies.CountActive(), 0u);
    ASSERT_EQ(player_bullets.CountActive(), 0u);
}
```

### ゲームプレイ再現(Game Like)

実際にいくつかの操作手順を事前に定義し，その通り操作する．

例：

1. ゲーム起動確認
2. プレイヤーの操作を実施
   - 全方向移動
   - 全方向低速移動
   - 射撃
3. 15秒程度項番2を無作為に繰り返す

## まとめ

複合試験では確認したいことを端的にまとめると，60FPSを維持できるかである．
16.67msを超えないかを調べることが重要である．

ただし，実ゲームでは背景やサウンド・弾幕パターンの計算やUIの描画など，多様な視覚的な処理が加わるので，少なくとも25%程度のバッファは見積もっておきたい．
そのため，実際には12.5ms程度で1Tickが完了するのが理想である．
