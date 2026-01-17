# 環境構築

## 必要なツール

### PlatformIO
- **推奨**: VS Code + PlatformIO Extension
- **代替**: PlatformIO Core (CLI)

### Git
- バージョン管理用
- GitHub連携

### シリアルモニタ（オプション）
- Arduino IDE付属のシリアルモニタ
- screen, minicom など

## PlatformIOのインストール

### VS Code + PlatformIO Extension（推奨）

1. VS Codeをインストール
2. Extensions から "PlatformIO IDE" をインストール
3. 再起動

### PlatformIO Core (CLI)

```bash
# Python 3.6+が必要
pip install platformio

# インストール確認
pio --version
```

## リポジトリのセットアップ

### クローン

```bash
git clone https://github.com/ixbox/1703-project.git
cd 1703-project
```

### ディレクトリ構成の確認

```bash
tree -L 2
```

## 最初のビルド

### Phase 0テストを試す

```bash
cd firmware/phase0-tests/pwm-basic
pio run
```

### Arduino UNO R3に書き込み

```bash
# ボードを接続
pio device list  # ポート確認

# アップロード
pio run --target upload

# シリアルモニタ
pio device monitor
```

## トラブルシューティング

### ポートが見つからない
- USBケーブルの確認（データ転送対応のもの）
- ドライバのインストール（CH340などクローンボードの場合）
- `pio device list`で確認

### ビルドエラー
- `platformio.ini`の確認
- `pio lib install`でライブラリインストール

### 書き込みエラー
- ボードの選択確認
- ブートローダーの確認
- 他のアプリケーションがポートを使用していないか確認

## 次のステップ

1. [development.md](development.md) でディレクトリ構成を理解
2. [roadmap.md](roadmap.md) で実装の流れを確認
3. Phase 0のテストを順番に試す
4. motor-controlの開発開始

## 参考リンク

- [PlatformIO Getting Started](https://docs.platformio.org/en/latest/integration/ide/vscode.html)
- [Arduino UNO R3 Documentation](https://docs.arduino.cc/hardware/uno-rev3)
