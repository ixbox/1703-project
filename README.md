# 1703 Project

5インチゲージ蒸気機関車型車両（1703）のプロジェクト

## Overview

30年前に製作された手作り5インチゲージ車両の制御系統を、マイコンベースのモダンなシステムに刷新。

## Features

- Arduino Nano + BTS7960によるPWM制御
- ウォームギア駆動（1:41）、駆動輪径89.5mm
- シリアルコマンドインターフェース
- 安全な方向転換制御
- 電流モニタリング
- (Phase 2) ESP32によるBLE制御

## Documentation

詳細は[docs/README.md](docs/README.md)を参照

## Hardware

- MCU: Arduino Nano (ATmega328P)
- Motor Driver: BTS7960
- Motor: RS-540SH
- Power: 7.2V NiMH Battery
- Gear Ratio: 1:41 (Worm Gear)
- Wheel Diameter: 89.5mm

## Development

- Platform: PlatformIO
- Language: C++ (Arduino Framework)

## Project Structure
```
firmware/motor-control/  - モーター制御MCU用ファームウェア
firmware/ui-control/     - UI MCU用ファームウェア（Phase 2）
docs/                    - プロジェクトドキュメント
common/                  - 共通定義ファイル
tools/                   - 開発ツール
```

## License

MIT