# 制御理論

## PWM制御

### 周波数設定
- **目標**: 8-12kHz
- **理由**: 可聴域外、モーター制御に適切、UNO R3で実装可能

### Timer1設定（10kHz）

```cpp
void setup() {
  // Timer1を10kHz PWMに設定
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // プリスケーラなし
  
  // TOP値設定: Fpwm = Fclk / (TOP + 1) → TOP = 16MHz / 10kHz - 1 = 1599
  ICR1 = 1599;

  // 初期duty 0%
  OCR1A = 0; // D9 (RPWM)
  OCR1B = 0; // D10 (LPWM)
}
```

### 基本制御関数

```cpp
// ピン定義
#define RPWM_PIN 9
#define LPWM_PIN 10
#define R_EN_PIN 11
#define L_EN_PIN 12

void setup() {
  // Timer1を10kHz PWMに設定
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
  ICR1 = 1599;
  OCR1A = 0;
  OCR1B = 0;

  // EN端子の初期化（両方HIGH固定、方向はRPWM/LPWMで制御）
  pinMode(R_EN_PIN, OUTPUT);
  pinMode(L_EN_PIN, OUTPUT);
  digitalWrite(R_EN_PIN, HIGH);
  digitalWrite(L_EN_PIN, HIGH);
}

// 前進: RPWMにPWM、LPWM=0
void forward(uint8_t speed) {  // speed: 0-255
  uint16_t duty = constrain(map(speed, 0, 255, 0, ICR1), 0, ICR1);
  OCR1A = duty; // RPWM
  OCR1B = 0;    // LPWM
}

// 後退: RPWM=0、LPWMにPWM
void backward(uint8_t speed) {
  uint16_t duty = constrain(map(speed, 0, 255, 0, ICR1), 0, ICR1);
  OCR1A = 0;    // RPWM
  OCR1B = duty; // LPWM
}

// 停止（コースト: 両PWM=0、EN維持）
void stop() {
  OCR1A = 0;
  OCR1B = 0;
}

// 緊急停止（EN=LOWでハードウェア的にモーター回路を遮断）
void emergencyStop() {
  OCR1A = 0;
  OCR1B = 0;
  digitalWrite(R_EN_PIN, LOW);
  digitalWrite(L_EN_PIN, LOW);
}
```

## 方向転換制御

### 安全な方向転換の要件

ウォームギアのバックラッシュを考慮し、**必ず完全停止を挟む**制御が必須。

### 状態マシン実装

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
  } else if(currentState == FORWARD) {
    // 前進中は速度変更のみ
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
  } else if(currentState == BACKWARD) {
    // 後退中は速度変更のみ
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

### 実装時の検討事項
- `STOP_WAIT_TIME`は実機で測定（PWM=0から完全停止までの時間）
- 電流値（R_IS/L_IS）による停止確認を推奨（電流が閾値以下で停止判定とする方が再現性が高い）
- もしくは「PWM=0 → 数msデッドタイム → 切替」で十分か実機で検証
- 連続した方向転換コマンドのデバウンス処理

## 通信プロトコル

### Phase 1: シリアルコマンド

**コマンドフォーマット（シンプルなASCII）**:
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

**実装例**:
```cpp
// 行バッファによる非同期パース（制御ループをブロックしない）
char cmdBuf[16];
uint8_t cmdLen = 0;

void setup() {
  Serial.begin(9600);  // ノイズ耐性のため低速スタート
  // PWM, BTS7960の初期化
}

void loop() {
  updateMotorState();

  // 1文字ずつ読み取り、改行で処理（ノンブロッキング）
  while(Serial.available() > 0) {
    char c = Serial.read();
    if(c == '\n' || c == '\r') {
      if(cmdLen > 0) {
        cmdBuf[cmdLen] = '\0';
        processCommand(cmdBuf);
        cmdLen = 0;
      }
    } else if(cmdLen < sizeof(cmdBuf) - 1) {
      cmdBuf[cmdLen++] = c;
    }
  }
}

void processCommand(const char* buf) {
  char cmd = buf[0];
  uint8_t speed = (buf[1] != '\0') ? atoi(&buf[1]) : 0;

  switch(cmd) {
    case 'F': requestForward(speed); break;
    case 'B': requestBackward(speed); break;
    case 'S': stop(); currentState = STOPPED; break;
    case 'M': sendStatus(); break;
  }
}

void sendStatus() {
  uint16_t current = readCurrent();
  Serial.print("I:");
  Serial.println(current);
}
```

### Phase 2: UART通信仕様

- **ボーレート**: 9600-19200 bps（ノイズ耐性重視、実測で調整）
- **レベルシフト**: 5V（Arduino Nano）↔ 3.3V（ESP32など）
- **プロトコル**: Phase 1と同一のASCIIコマンド

### 将来的な拡張

**バイナリプロトコル案**:
```
[STX(0x02)][CMD][LEN][DATA...][CHK][ETX(0x03)]
```

詳細は実装時に検討。
