# 開発ガイド

## ディレクトリ構成

```
1703-project/
├── README.md                   # プロジェクト全体の説明
├── LICENSE
├── docs/                       # ドキュメント
│   ├── README.md              # ドキュメント目次
│   ├── overview.md
│   ├── vehicle-specs.md
│   ├── system-design.md
│   ├── hardware.md
│   ├── control-theory.md
│   ├── development.md         # このファイル
│   ├── getting-started.md
│   ├── roadmap.md
│   └── bom.md
├── firmware/
│   ├── phase0-tests/          # 実験・検証エリア
│   │   ├── pwm-basic/
│   │   │   ├── platformio.ini
│   │   │   ├── src/
│   │   │   │   └── main.cpp
│   │   │   └── README.md
│   │   ├── bts7960-control/
│   │   ├── current-monitor/
│   │   ├── display-test/
│   │   └── serial-command/
│   ├── motor-control/         # Phase 1: 本番コード
│   │   ├── platformio.ini
│   │   ├── src/
│   │   │   └── main.cpp
│   │   ├── include/
│   │   │   ├── motor_control.h
│   │   │   ├── command_parser.h
│   │   │   └── current_monitor.h
│   │   └── lib/
│   └── ui-control/            # Phase 2: UI MCU用
│       ├── platformio.ini
│       ├── src/
│       └── include/
├── common/                     # 両MCUで共有する定義
│   ├── protocol.h
│   └── constants.h
└── tools/
    └── serial-test.py
```

## phase0-testsの位置づけ

### 目的
- 実験・検証・学習のための自由なエリア
- 新しい機能やアイデアを試す場所
- 動作確認の記録

### 特徴
- 各テストが完全に独立したPlatformIOプロジェクト
- 番号付けは任意（わかりやすければ何でもOK）
- 順番も自由
- 後から追加・削除が容易

### 使い方

**新しいテストの追加**:
```bash
cd firmware/phase0-tests
mkdir my-new-test
cd my-new-test
pio project init --board uno
# コードを書く
```

**README.mdの記録**:
各テストディレクトリにREADME.mdを作成し、以下を記録：
- テストの目的
- 使用ハードウェア
- 結果・測定データ
- 気づいたこと
- 次のステップ

### phase0-testsとmotor-controlの違い

| | phase0-tests | motor-control |
|---|---|---|
| 位置づけ | 実験ノート | 本番コード |
| 目的 | 検証・学習 | 車両を動かす |
| 品質 | 動けばOK | プロダクション品質 |
| 変更頻度 | 高い | 低い（慎重） |

## 開発の流れ

### Step 1: phase0で検証
1. 新しい機能のアイデア
2. phase0-testsで小さく実験
3. 動作確認とREADME記録

### Step 2: motor-controlに統合
1. phase0で確認できた機能
2. motor-controlに実装
3. 他の機能との統合テスト

### Step 3: 実機テスト
1. 車両に搭載
2. 動作確認
3. 問題があればphase0に戻る

## コーディング規約

### ファイル命名
- ヘッダー: `snake_case.h`
- ソース: `snake_case.cpp`
- 定数定義: `UPPER_SNAKE_CASE`

### コメント
- 重要な判断理由を記録
- なぜそうしたのかを書く
- コードを読めばわかることは書かない

### Git コミット
- わかりやすいコミットメッセージ
- 1コミット1機能
- phase0-testsは気軽にコミット

## PlatformIOの使い方

### プロジェクトのビルド
```bash
cd firmware/motor-control
pio run
```

### アップロード
```bash
pio run --target upload
```

### シリアルモニタ
```bash
pio device monitor
```

### 環境の切り替え
```bash
pio run -e nano  # Nano環境でビルド
```

## トラブルシューティング

### ビルドエラー
1. `platformio.ini`の設定確認
2. ライブラリの依存関係確認
3. `pio lib list`で確認

### アップロードエラー
1. ポートの確認: `pio device list`
2. ボードの選択確認
3. USBケーブルの確認

### phase0で試してみる
- 問題を最小構成で再現
- phase0で切り分け
- 動いたら本番に反映

## 参考リンク

- [PlatformIO Documentation](https://docs.platformio.org/)
- [Arduino Reference](https://www.arduino.cc/reference/en/)
- [ATmega328P Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)
