# 5インチゲージ車両 制御システム改修プロジェクト

## プロジェクト概要

30年前に製作された手作り5インチゲージ車両（1703）の制御系統をマイコンベースのモダンなシステムに刷新するプロジェクト。

### 車両仕様
- 型式: 1703（蒸気機関車型）
- ゲージ: 5インチ
- 用途: 展示走行（乗車は想定しない）
- 駆動方式: 電動モーター駆動
- 駆動機構: ウォームギア（手作り）で駆動輪を回転
  - ギア比: 1:41（モーター1回転で駆動輪1/41回転）
  - 駆動輪径: 89.5mm（円周: 約281mm）
  - 特性: 自己保持性（セルフロック）あり、バックラッシュの存在

## 現状の確認事項

### 既存システムの状態

**電気系統**
- バッテリー: バイク用鉛蓄電池（電圧0.1V、使用不可）
- モーター: RS-540SH（5V印加で動作確認済み）
- スイッチ類: 導通不良により交換が必要
- 配線: 30年の経年劣化あり

**制御系統**
- コントローラーボックス: LOW SPEED/HIGH、FW/STOP/BW スイッチ
- 3速切替のインジケーター
- 内部配線: 赤・白・緑線など複数確認

## システム設計

### 開発環境

**PlatformIO採用**

複数MCUの管理と将来の拡張性を考慮し、Arduino IDEではなくPlatformIOを使用。

**メリット**:
- 複数MCU（UNO R3, Nano, ESP32）の統一管理
- ライブラリのバージョン管理
- プロジェクト構造の整理（src/, include/, lib/）
- Phase 2でのMCU分割時に有利
- デバッグとテストの強化

**想定プロジェクト構成**:
```
sl-controller/
├── platformio.ini          # プロジェクト設定
├── src/
│   ├── main.cpp           # メインループ
│   ├── motor_control.cpp  # モーター制御
│   ├── command_parser.cpp # シリアルコマンド処理
│   └── current_monitor.cpp # 電流モニタリング
├── include/
│   ├── motor_control.h
│   ├── command_parser.h
│   └── current_monitor.h
├── lib/                   # プロジェクト固有のライブラリ
└── test/                  # ユニットテスト（オプション）
```

**platformio.ini例**:
```ini
[platformio]
default_envs = uno

[env:uno]
platform = atmelavr
board = uno
framework = arduino
monitor_speed = 9600

[env:nano]
platform = atmelavr
board = nanoatmega328
framework = arduino
monitor_speed = 9600

[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

### 開発フェーズ

**Phase 1: 基本動作確立**
- Arduino UNO R3単体でPWM制御
- BTS7960によるモーター駆動
- シリアルコマンドによる制御

**Phase 2: UI分離（将来構想）**
- モーター制御MCUをArduino Nanoに変更
- UI用MCU追加（ESP32想定）
- UART通信でモーター制御MCUと分離
- BLE経由でのスマートフォン操作

### 電源系統

```
7.2V系統（モーター駆動用）
├─ タミヤ 7.2V NiMHバッテリー
├─ 10Aヒューズ
├─ メインスイッチ（キルスイッチ兼用）
└─ BTS7960 → RS-540SH

5V系統（制御系用）
└─ モバイルバッテリー → Arduino UNO R3 / Nano

3.3V系統（将来追加）
└─ UI MCU用（ESP32など）
```

### MCU選定の詳細

#### Phase 1: Arduino UNO R3
- **用途**: プロトタイピングとデバッグ
- **理由**: 
  - ブレッドボード実装が容易
  - USB直結でシリアル通信が簡単
  - デバッグ時の配線変更が柔軟

#### Phase 2: Arduino Nano (ATmega328P)
- **用途**: 最終的な車載実装（モーター制御MCU）
- **理由**: 
  - UNO R3とピン互換（コード移植不要）
  - 小型で車載に適したフォームファクタ
  - 十分な処理能力とペリフェラル
  - 開発環境の成熟度が高い
  - 豊富なライブラリとコミュニティサポート

#### ATtiny1616を採用しない理由
本プロジェクトでは省電力性よりも開発効率と拡張性を優先する判断：

- **省電力性が活かせない**
  - 走行時は常時モーター駆動（数Aオーダーの消費）
  - 制御MCUの消費電力（数十mA）は全体の誤差範囲
  - 常時稼働が前提で、スリープモードを使う機会がない

- **開発効率の優先**
  - ATmega328Pの方が開発環境が成熟
  - 豊富なライブラリとサンプルコード
  - デバッグツールが充実

- **将来の拡張性**
  - ピン数に余裕があり、センサー追加などに対応可能
  - メモリ容量に余裕（Flash 32KB, SRAM 2KB）

**参考**: ATtiny1616が有効な用途例
- 電池駆動で長期間待機するセンサーノード
- 間欠動作が主体のIoTデバイス
- スペースが極端に制約される組み込み機器

### ハードウェア構成

#### モータードライバ: BTS7960
- 最大電流: 43A
- PWM入力: RPWM, LPWM（独立制御）
- イネーブル: R_EN, L_EN（Arduino制御）
- 電流センス: R_IS, L_IS
- ロジック電源: 5V
- モーター電源: 7.2V

**配線仕様**:
```
BTS7960          Arduino UNO R3/Nano
----------------------------------------
RPWM       →     D9 (Timer1 PWM)
LPWM       →     D10 (Timer1 PWM)
R_EN       →     D7 (Digital Out)
L_EN       →     D8 (Digital Out)
R_IS       →     A0 (Analog In)
L_IS       →     A1 (Analog In)
VCC        →     5V
GND        →     GND
B+         →     7.2V (Battery+)
B-         →     Battery-
M+         →     Motor+
M-         →     Motor-
```

**EN端子制御方式の採用理由**:
- ハードウェア的にモーター回路を切断可能（安全性向上）
- 緊急停止時の確実な動作保証
- 方向転換時の誤動作防止

#### 電流センサ

**BTS7960内蔵センサ（R_IS/L_IS）**
- 電流比: 約1/8500
- 用途: Phase 1での電流モニタリング
- 出力: アナログ電圧（Arduino ADCで読み取り）
- 測定対象: 各方向の電流を独立して計測可能

**ACS758LCB-050B（Phase 2以降で検討）**
- 測定範囲: ±50A
- 感度: 40mV/A
- 出力中点: 2.5V（VCC=5V時）
- 配置案: ローサイド
- 特徴: 絶縁型、双方向計測、高精度
- 備考: Phase 1では実装せず、より精密な電流計測が必要になった場合に追加検討

#### 電流センサ配置（Phase 1）
```
[Battery+] → [Fuse] → [Main SW] → [BTS7960] → [Motor] → [Battery-]
                                        ↓
                                   R_IS, L_IS
                                        ↓
                                   Arduino ADC
```

**Phase 2以降での拡張案**:
```
[Battery+] → [Fuse] → [Main SW] → [BTS7960] → [Motor] → [ACS758] → [Battery-]
                                        ↓                      ↓
                                   R_IS, L_IS            Arduino ADC
                                        ↓
                                   Arduino ADC
```

#### 筐体内配置（物理レイアウト）
```
[筐体内配置（推定）]

[バッテリー] → [ヒューズ] → [スイッチ]
                              ↓
                        [BTS7960]
                          M+  M-
                           ↓
                        [モーター]
                       （ウォームギア）
                           ↓
                        [駆動輪]
```

**配置の考慮点**
- BTS7960をモーターの近くに配置し、配線を短縮
- 発熱部品（BTS7960、モーター）は放熱を考慮して配置
- 実際の配置は既存の筐体構造と重量バランスを考慮して決定

### 配線材料の選定

#### 7.2V系（モーター駆動系）
**ケーブル**
- カーオーディオ用 1.25sq（AWG16相当）
- 理由：
  - 調達が容易（カー用品店で入手可）
  - 許容電流: 約10-15A（十分な余裕）
  - 柔軟性があり車両内配線に適している

**コネクタ**
- ギボシ端子（250型）
- ファストン端子（6.3mm）
- 理由：
  - 汎用性が高く入手容易
  - 工具不要で着脱可能（メンテナンス性向上）
  - 各種部品との接続が容易
  - 振動に対する信頼性

#### 5V系（制御系）
- 細線（AWG22-24程度）
- デュポンコネクタ、XHコネクタなど
- 信号線は個別に絶縁されたリボンケーブルも検討

### PWM制御仕様

#### ウォームギア駆動の特性と制御への影響

**ウォームギアの特性**:
- 自己保持性（セルフロック）: 駆動輪からモーターへの逆回転が困難
- 減速比が大きい: 詳細な比率は実測予定
- 効率が低い: 平ギアと比較して伝達効率が低い
- バックラッシュ: 手作りのため遊びが存在

**制御への影響**:

1. **停止特性**
   - 自己保持性により、PWM=0で即座に停止
   - 慣性による惰行がほとんどない
   
2. **方向転換時の注意**
   - バックラッシュにより、前進→後退の切り替え時に一瞬空転する可能性
   - **必ず完全停止を挟む制御が必須**
   
3. **起動電流**
   - 静止摩擦とギア効率の低さにより、起動時の電流が大きい
   - ソフトスタートの実装が重要

4. **速度計算の参考値**
   - RS-540SHの無負荷回転数: 約20,000 rpm（7.2V時）
   - ギア比1:41を考慮: 駆動輪回転数 ≒ 488 rpm ≒ 8.1 rps
   - 駆動輪円周: 約281mm
   - 理論最高速度: 約2.3 m/s（実際は負荷により低下）
   - PWM dutyと速度の関係は実測して確認

#### 方向転換時の制御シーケンス

**安全な方向転換の実装例**:
```cpp
enum MotorState {
  STOPPED,
  FORWARD,
  BACKWARD,
  STOPPING  // 停止中の遷移状態
};

MotorState currentState = STOPPED;
MotorState targetState = STOPPED;
uint8_t targetSpeed = 0;
unsigned long stopStartTime = 0;
const unsigned long STOP_WAIT_TIME = 100;  // ms, 実測して調整

void requestForward(uint8_t speed) {
  if(currentState == BACKWARD) {
    // 後退中なので、まず停止
    currentState = STOPPING;
    targetState = FORWARD;
    targetSpeed = speed;
    stop();
    stopStartTime = millis();
  } else if(currentState == STOPPED) {
    // 停止中なので即座に前進
    forward(speed);
    currentState = FORWARD;
  }
  // FORWARD中の場合は速度変更のみ
  else if(currentState == FORWARD) {
    forward(speed);
  }
}

void requestBackward(uint8_t speed) {
  if(currentState == FORWARD) {
    // 前進中なので、まず停止
    currentState = STOPPING;
    targetState = BACKWARD;
    targetSpeed = speed;
    stop();
    stopStartTime = millis();
  } else if(currentState == STOPPED) {
    // 停止中なので即座に後退
    backward(speed);
    currentState = BACKWARD;
  }
  // BACKWARD中の場合は速度変更のみ
  else if(currentState == BACKWARD) {
    backward(speed);
  }
}

void updateMotorState() {
  if(currentState == STOPPING) {
    // 停止時間経過をチェック
    if(millis() - stopStartTime >= STOP_WAIT_TIME) {
      currentState = STOPPED;
      
      // 目標状態へ遷移
      if(targetState == FORWARD) {
        forward(targetSpeed);
        currentState = FORWARD;
      } else if(targetState == BACKWARD) {
        backward(targetSpeed);
        currentState = BACKWARD;
      }
      
      targetState = STOPPED;
    }
  }
}

void loop() {
  updateMotorState();
  // その他の処理
}
```

**実装時の検討事項**:
- `STOP_WAIT_TIME`は実機で測定（PWM=0から完全停止までの時間）
- 電流値による停止確認も検討（電流が閾値以下になるまで待つ）
- 連続した方向転換コマンドのデバウンス処理

#### PWM周波数
- 目標: 8-12kHz
- 理由: 可聴域外、モーター制御に適切、UNO R3で実装可能

#### Timer1設定例（10kHz）
```cpp
void setup() {
  // Timer1を10kHz PWMに設定
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // プリスケーラなし
  
  // TOP値設定（16MHz / 10kHz = 1600）
  ICR1 = 1600;
  
  // 初期duty 0%
  OCR1A = 0; // D9 (RPWM)
  OCR1B = 0; // D10 (LPWM)
}

void setPWM(uint8_t pin, uint16_t duty) {
  // duty: 0-1600の範囲
  if(pin == 9) {
    OCR1A = duty;
  } else if(pin == 10) {
    OCR1B = duty;
  }
}
```

#### 基本制御関数
```cpp
// ピン定義
#define RPWM_PIN 9
#define LPWM_PIN 10
#define R_EN_PIN 7
#define L_EN_PIN 8

void setup() {
  // Timer1を10kHz PWMに設定
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
  ICR1 = 1600;
  OCR1A = 0;
  OCR1B = 0;
  
  // EN端子の初期化
  pinMode(R_EN_PIN, OUTPUT);
  pinMode(L_EN_PIN, OUTPUT);
  digitalWrite(R_EN_PIN, LOW);
  digitalWrite(L_EN_PIN, LOW);
}

// 前進
void forward(uint8_t speed) {  // speed: 0-255
  uint16_t duty = map(speed, 0, 255, 0, 1600);
  
  // 方向設定
  digitalWrite(R_EN_PIN, HIGH);
  digitalWrite(L_EN_PIN, LOW);
  
  // PWM設定
  OCR1A = duty;   // RPWM
  OCR1B = 0;      // LPWM
}

// 後退
void backward(uint8_t speed) {
  uint16_t duty = map(speed, 0, 255, 0, 1600);
  
  // 方向設定
  digitalWrite(R_EN_PIN, LOW);
  digitalWrite(L_EN_PIN, HIGH);
  
  // PWM設定
  OCR1A = 0;      // RPWM
  OCR1B = duty;   // LPWM
}

// 停止
void stop() {
  // EN端子をLOWにしてハードウェア的に切断
  digitalWrite(R_EN_PIN, LOW);
  digitalWrite(L_EN_PIN, LOW);
  
  // PWMも0に
  OCR1A = 0;
  OCR1B = 0;
}
```

### 通信プロトコル

#### Phase 1: シリアルコマンド（内部エミュレーション）

**コマンドフォーマット（シンプルなASCII）**
```
'F' + 数値: 前進（速度0-255）
'B' + 数値: 後退（速度0-255）
'S': 停止
'M': モニタリングデータ要求

例:
F128  → 前進、速度128
B64   → 後退、速度64
S     → 停止
M     → ステータス送信
```

**実装例**
```cpp
void setup() {
  Serial.begin(9600);  // ノイズ耐性のため低速スタート
  // PWM, BTS7960の初期化
}

void loop() {
  if(Serial.available() > 0) {
    char cmd = Serial.read();
    
    switch(cmd) {
      case 'F': // 前進
        if(Serial.available() > 0) {
          uint8_t speed = Serial.parseInt();
          forward(speed);
        }
        break;
        
      case 'B': // 後退
        if(Serial.available() > 0) {
          uint8_t speed = Serial.parseInt();
          backward(speed);
        }
        break;
        
      case 'S': // 停止
        stop();
        break;
        
      case 'M': // モニタリング要求
        sendStatus();
        break;
    }
  }
}

void sendStatus() {
  uint16_t current = readCurrent();
  Serial.print("I:");
  Serial.println(current);
}
```

#### UART通信仕様

**Phase 2でのMCU間通信**
- ボーレート: 9600-19200 bps（ノイズ耐性重視、実測で調整）
- レベルシフト: 5V（Arduino Nano）↔ 3.3V（ESP32など）
- プロトコル: Phase 1と同一のASCIIコマンド

**将来的な拡張（オプション）**
```
バイナリプロトコル案:
[STX(0x02)][CMD][LEN][DATA...][CHK][ETX(0x03)]
```

## 責務分離

### モーター制御MCU（Arduino UNO R3 → Nano）
- PWM生成とモータードライバ制御
- ソフトスタート/ソフトブレーキ
- 電流モニタリング（BTS7960 IS + ACS758）
- 過電流保護などの安全機能
- 受信コマンドの実行
- 低レベル制御の確実な動作保証

### UI MCU（Phase 2: ESP32など）
- ユーザー入力の受付（物理スイッチ or BLE）
- 表示系（速度表示、電流表示など）
- 上位レベルの制御ロジック（ノッチ制御など）
- モーター制御MCUへのコマンド送信
- 柔軟な機能拡張

## 実装ロードマップ

### Phase 1: 基本動作確立（Arduino UNO R3）

#### Step 0: 初期検証（Nゲージモーター使用）
- **目的**: 制御回路の動作確認（PWM制御とBTS7960の動作確認が主目的）
- **使用モーター**: Nゲージ用小型DCモーター（手元在庫）
  - 定格電圧: 12V
  - 耐圧: 12Vまで対応
  - 5V動作: 定格以下だが動作可能（トルク・回転数は低下）
- **電源構成**:
  - 5Vモバイルバッテリー（手元在庫を使用）
  - VCC（BTS7960ロジック電源）: 5V
  - B+（BTS7960モーター電源）: 5V
- **検証条件**:
  - 無負荷での動作確認
  - 電圧は定格以下だが検証目的には問題なし
- **検証項目**:
  - BTS7960の動作確認（RPWM/LPWM制御）
  - PWM周波数設定と動作確認
  - シリアルコマンドの動作確認
  - 前進/後退/停止の切り替え確認
  - 電流センサの読み取り確認（低電流だが動作原理の確認）
- **利点**:
  - 低電流・低電圧で安全に動作確認可能
  - RS-540SHを使う前に制御ロジック全体をデバッグ完了
  - 配線ミスによる部品損傷リスクの低減
  - 7.2Vバッテリー到着前に開発を進められる

#### Step 1: PWM制御とBTS7960の基本動作
- Timer1を使った10kHz PWM生成
- **前進方向のみで動作確認**（まずは一方向で確実に）
- 固定速度での動作確認
- モーターの回転確認
- EN端子の動作確認

#### Step 2: シリアルコマンドで速度可変
- ASCIIコマンドの実装
- シリアルモニタからの動作確認
- 前進方向での速度0-255の全範囲確認
- **ソフトスタートの実装**
  - 起動時の電流急増を抑制
  - PWM dutyの段階的な上昇
  - 加速カーブの調整
- **後退方向の動作確認**
  - 前進が安定してから後退を追加
  - 後退方向での速度制御確認
- **方向転換時の安全制御の実装**（Step 2の最後）
  - 状態マシンによる方向管理
  - 前進↔後退切り替え時の強制停止待機
  - 停止時間の実測と調整

#### Step 3: 電流モニタリング
- BTS7960 R_IS/L_IS出力による電流計測
- ADC値から電流値への変換式の確立
- シリアル出力での電流値表示
- 無負荷時と負荷時の電流観察
- 前進/後退での電流値の比較
- 起動時の突入電流の測定

#### Step 4: 安全機能
- 過電流検知と保護動作
- ストール検出
- 異常時の自動停止

### Phase 2: UI分離とBLE制御（将来構想）

#### ハードウェア移行
- モーター制御MCUをArduino Nanoに変更
- 車載用基板の設計と実装
- 配線の最適化

#### UI MCU実装
- UI MCUの選定と実装（ESP32想定）
- UART通信の確立とレベルシフト
- BLEによるスマートフォン操作
- 物理UIの実装（ディスプレイ、スイッチなど）

### 将来的な拡張構想

#### 電車的な制御
- ノッチ制御（力行1-5段、制動1-8段など）
- マスコン風の操作感
- 加速度制限（急加速防止）
- 惰行制御（ノッチオフ時の滑走）
- 回生ブレーキ風の減速カーブ

#### モニタリング機能強化
- 現在電流値のリアルタイム表示
- PWM duty比の表示
- 走行ログの記録
- エンコーダーによる速度フィードバック（検討中）

## 安全設計

### 電気的保護
- 10Aヒューズ（ハイサイド）
- メインスイッチ（キルスイッチ兼用）
- 過電流検知とソフトウェア保護

### ソフトウェア保護
- ストール検出（電流急増の監視）
- 異常時の自動停止
- ソフトスタート/ソフトブレーキ

## 部品リスト

### 確定部品

**制御系**
- Arduino UNO R3（Phase 1プロトタイピング用）
- Arduino Nano（Phase 2車載実装用）
- BTS7960 モータードライバ

**動力系**
- RS-540SH モーター（既存、本番用）
- Nゲージ用モーター（初期検証用）
- タミヤ 7.2V NiMHバッテリー
- ウォームギア（既存、手作り）

**電源・保護**
- モバイルバッテリー（5V制御系電源）
- 10Aヒューズ
- メインスイッチ

**配線材料**
- カーオーディオ用ケーブル 1.25sq（7.2V系）
- ギボシ端子 250型
- ファストン端子 6.3mm
- 細線 AWG22-24（5V系）
- デュポンコネクタ / XHコネクタ

### Phase 2以降で追加予定

**UI系**
- ESP32開発ボード（UI MCU）
- レベルシフトIC（5V↔3.3V UART通信用）
- ディスプレイ（仕様未定）
- 物理スイッチ類（仕様未定）

**電流センサ（オプション）**
- ACS758LCB-050B（より精密な電流計測が必要な場合）

## 備考

- 既存配線は30年の経年劣化があるため、制御系刷新に合わせて配線も全面的に更新予定
  - ただし、モーターと制御盤間のU字ターミナル端子板は状態次第で流用可能
- ノイズ対策としてPWM周波数やUARTボーレートは実測して調整
- Phase 1の動作確認を優先し、段階的に機能を追加していく方針
- UNO R3からNanoへの移行は、コードの互換性が保たれるため容易
