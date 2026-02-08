/**
 * 03-motor-driver-test: BTS7960モータードライバーテスト
 *
 * 契約プログラミングに基づく設計:
 *   Physical - ピン定義、ハードウェア初期化
 *   Logical  - MotorDriver契約、SerialCommand契約
 *   Meaning  - loopには意味だけ
 *
 * 配線 (docs/hardware.md準拠):
 *   D9  (OC1A) -> RPWM (正転PWM)
 *   D10 (OC1B) -> LPWM (逆転PWM)
 *   D11        -> R_EN (右イネーブル)
 *   D12        -> L_EN (左イネーブル)
 *   VCC        -> 5V
 *   GND        -> GND
 */

#include <Arduino.h>
#include <PwmOutput.h>
#include <MotorDriver.h>

// ============================================================
// Physical Layer - ボード固有の定義
// ============================================================

namespace Board
{
    // BTS7960接続ピン (docs/hardware.md準拠)
    constexpr uint8_t RPWM_PIN = 9;  // Timer1 OC1A
    constexpr uint8_t LPWM_PIN = 10; // Timer1 OC1B
    constexpr uint8_t R_EN_PIN = 11; // Digital Out
    constexpr uint8_t L_EN_PIN = 12; // Digital Out
}

namespace PwmConfig
{
    constexpr uint16_t FREQUENCY_HZ = 12000; // 12kHz (モーター駆動に適した周波数)
}

// ============================================================
// Logical Layer - 契約の定義
// ============================================================

struct SerialCommand
{
    bool hasCommand()
    {
        return Serial.available() > 0;
    }

    String take()
    {
        String input = Serial.readStringUntil('\n');
        input.trim();
        return input;
    }
};

// ============================================================
// Meaning Layer - コマンド定義
// ============================================================

namespace Command
{
    constexpr char FORWARD = 'f';  // 正転
    constexpr char REVERSE = 'r';  // 逆転
    constexpr char STOP = 's';     // 停止
    constexpr char BRAKE = 'b';    // ブレーキ
    constexpr char ENABLE = 'e';   // イネーブル
    constexpr char DISABLE = 'd';  // ディスエーブル
    constexpr char INCREASE = '+'; // 速度増加
    constexpr char DECREASE = '-'; // 速度減少
    constexpr char STATUS = '?';   // 状態表示
}

// ============================================================
// Setup - 結線
// ============================================================

// Timer1設定（12kHz）
PwmOutput::Timer1Config timer1Config = PwmOutput::Timer1Config::fromFrequency(PwmConfig::FREQUENCY_HZ);

// PWMチャンネル
PwmOutput::PwmChannel rpwm = {PwmOutput::Channel::A, timer1Config};
PwmOutput::PwmChannel lpwm = {PwmOutput::Channel::B, timer1Config};

// モータードライバー
MotorDriver::Bts7960 motor = {rpwm, lpwm, Board::R_EN_PIN, Board::L_EN_PIN, 0};

// コマンド入力
SerialCommand cmd;

void printStatus()
{
    Serial.print(F("Speed: "));
    Serial.print(motor.speed());
    Serial.print(F("% | Enabled: "));
    Serial.print(motor.isEnabled() ? F("Yes") : F("No"));
    Serial.print(F(" | RPWM: "));
    Serial.print(rpwm.percent());
    Serial.print(F("% | LPWM: "));
    Serial.print(lpwm.percent());
    Serial.println(F("%"));
}

void printHelp()
{
    Serial.println(F("=== BTS7960 Motor Driver Test ==="));
    Serial.print(F("PWM Frequency: "));
    Serial.print(PwmConfig::FREQUENCY_HZ);
    Serial.println(F("Hz"));
    Serial.println(F("Commands:"));
    Serial.println(F("  -100~100 : Set speed (negative=reverse)"));
    Serial.println(F("  f        : Forward 50%"));
    Serial.println(F("  r        : Reverse 50%"));
    Serial.println(F("  s        : Stop (coast)"));
    Serial.println(F("  b        : Brake (short)"));
    Serial.println(F("  e        : Enable driver"));
    Serial.println(F("  d        : Disable driver"));
    Serial.println(F("  +        : Increase speed by 10%"));
    Serial.println(F("  -        : Decrease speed by 10%"));
    Serial.println(F("  ?        : Show status"));
    Serial.println();
}

void setup()
{
    Serial.begin(9600);

    // Timer1初期化（PWMチャンネルより先に）
    timer1Config.apply();

    // モータードライバー初期化
    motor.begin();
    motor.enable();

    printHelp();
    printStatus();
}

// ============================================================
// Loop - 意味だけを書く
// ============================================================

void loop()
{
    if (cmd.hasCommand())
    {
        String input = cmd.take();

        if (input.length() == 0)
        {
            return;
        }

        char c = input[0];

        if (c == Command::FORWARD)
        {
            motor.setSpeed(50);
            Serial.println(F("Forward 50%"));
            printStatus();
        }
        else if (c == Command::REVERSE)
        {
            motor.setSpeed(-50);
            Serial.println(F("Reverse 50%"));
            printStatus();
        }
        else if (c == Command::STOP)
        {
            motor.stop();
            Serial.println(F("Stop"));
            printStatus();
        }
        else if (c == Command::BRAKE)
        {
            motor.brake();
            Serial.println(F("Brake"));
            printStatus();
        }
        else if (c == Command::ENABLE)
        {
            motor.enable();
            Serial.println(F("Enabled"));
            printStatus();
        }
        else if (c == Command::DISABLE)
        {
            motor.disable();
            Serial.println(F("Disabled"));
            printStatus();
        }
        else if (c == Command::INCREASE)
        {
            int8_t newSpeed = constrain(motor.speed() + 10, -100, 100);
            motor.setSpeed(newSpeed);
            printStatus();
        }
        else if (c == Command::DECREASE)
        {
            int8_t newSpeed = constrain(motor.speed() - 10, -100, 100);
            motor.setSpeed(newSpeed);
            printStatus();
        }
        else if (c == Command::STATUS)
        {
            printStatus();
        }
        else if (c == '-' || isDigit(c))
        {
            int value = input.toInt();
            if (value >= -100 && value <= 100)
            {
                motor.setSpeed(value);
                printStatus();
            }
            else
            {
                Serial.println(F("Speed must be -100 to 100"));
            }
        }
    }
}
