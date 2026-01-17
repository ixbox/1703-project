# 1703 Project ドキュメント

祖父の手作り5インチゲージ蒸気機関車（1703）を動かすプロジェクトのドキュメント集

## ドキュメント一覧

### プロジェクト概要
- **[overview.md](overview.md)** - プロジェクトの背景、目的、開発フェーズ

### 技術仕様
- **[vehicle-specs.md](vehicle-specs.md)** - 車両の仕様（ギア比、輪径、駆動機構など）
- **[system-design.md](system-design.md)** - システム設計（MCU選定、電源系統、責務分離）
- **[hardware.md](hardware.md)** - ハードウェア詳細（配線、部品接続、筐体配置）
- **[control-theory.md](control-theory.md)** - 制御仕様（PWM、プロトコル、方向転換制御）

### 開発ガイド
- **[development.md](development.md)** - 開発ガイド（ディレクトリ構成、開発の進め方）
- **[getting-started.md](getting-started.md)** - 環境構築とセットアップ手順

### リファレンス
- **[roadmap.md](roadmap.md)** - 実装ロードマップ
- **[bom.md](bom.md)** - 部品表

## 読む順序の推奨

### 初めての方
1. [overview.md](overview.md) - プロジェクト全体像の把握
2. [getting-started.md](getting-started.md) - 環境構築
3. [development.md](development.md) - 開発の進め方

### 技術詳細を知りたい方
1. [vehicle-specs.md](vehicle-specs.md) - 車両の物理的な仕様
2. [system-design.md](system-design.md) - システム全体の設計思想
3. [hardware.md](hardware.md) - ハードウェアの接続方法
4. [control-theory.md](control-theory.md) - 制御アルゴリズムの詳細

### 実装を始める方
1. [development.md](development.md) - ディレクトリ構成の理解
2. [roadmap.md](roadmap.md) - 実装の段階的な計画
3. Phase 0のテストコードを参照
