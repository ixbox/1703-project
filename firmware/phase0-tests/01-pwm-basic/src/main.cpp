/**
 * 01-pwm-basic: PWM出力テスト
 *
 * 契約プログラミングに基づく設計:
 *   Physical - ピン定義、ハードウェア初期化
 *   Logical  - PwmOutput契約、SerialCommand契約
 *   Meaning  - loopには意味だけ
 */

#include <Arduino.h>

// ============================================================
// Physical Layer - ボード固有の定義
// ============================================================

namespace Board
{
    constexpr uint8_t PWM_PIN = 9; // Timer1 PWM (OC1A)
}

namespace PwmConfig
{
    // Timer1 Fast PWM 8-bit mode
    // 周波数 = 16MHz / (prescaler * 256)
    // prescaler=1: 62.5kHz, prescaler=8: 7.8kHz
    // Fast PWM with ICR1 top for custom frequency
    constexpr uint16_t FREQUENCY_HZ = 12000;                  // 10kHz target
    constexpr uint16_t ICR1_TOP = (F_CPU / FREQUENCY_HZ) - 1; // 1599 for 10kHz
}

// ============================================================
// Logical Layer - 契約の定義
// ============================================================

struct PwmOutput
{
    const uint8_t pin;
    uint16_t duty; // 0-1599 for 10kHz with ICR1=1599

    void begin()
    {
        pinMode(pin, OUTPUT);
        setupTimer1();
        apply();
    }

    void setupTimer1()
    {
        // Timer1: Fast PWM, ICR1 as TOP
        // WGM13:0 = 1110 (Fast PWM, TOP=ICR1)
        TCCR1A = (1 << COM1A1) | (1 << WGM11);
        TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // No prescaler
        ICR1 = PwmConfig::ICR1_TOP;
    }

    void set(uint8_t percent)
    {
        // 0-100 -> 0-ICR1_TOP にスケーリング
        duty = ((uint32_t)percent * PwmConfig::ICR1_TOP) / 100;
        apply();
    }

    void apply()
    {
        OCR1A = duty;
    }

    uint8_t percent() const
    {
        return (duty * 100) / PwmConfig::ICR1_TOP;
    }
};

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
// Meaning Layer - 意味の定義
// ============================================================

namespace Command
{
    constexpr char INCREASE = '+';
    constexpr char DECREASE = '-';
    constexpr char STATUS = '?';
}

// ============================================================
// Setup - 結線
// ============================================================

PwmOutput pwm = {Board::PWM_PIN, PwmConfig::ICR1_TOP / 2}; // 50%
SerialCommand cmd;

void printStatus()
{
    Serial.print(F("Duty: "));
    Serial.print(pwm.percent());
    Serial.print(F("% @ "));
    Serial.print(PwmConfig::FREQUENCY_HZ / 1000);
    Serial.println(F("kHz"));
}

void printHelp()
{
    Serial.println(F("=== PWM Basic Test ==="));
    Serial.print(F("PWM Frequency: "));
    Serial.print(PwmConfig::FREQUENCY_HZ);
    Serial.println(F("Hz"));
    Serial.println(F("Commands:"));
    Serial.println(F("  0-100 : Set duty cycle (%)"));
    Serial.println(F("  +     : Increase by 5%"));
    Serial.println(F("  -     : Decrease by 5%"));
    Serial.println(F("  ?     : Show current status"));
    Serial.println();
}

void setup()
{
    Serial.begin(9600);
    pwm.begin();

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

        if (input[0] == Command::INCREASE)
        {
            pwm.set(min(100, pwm.percent() + 5));
            printStatus();
        }
        else if (input[0] == Command::DECREASE)
        {
            pwm.set(max(0, pwm.percent() - 5));
            printStatus();
        }
        else if (input[0] == Command::STATUS)
        {
            printStatus();
        }
        else
        {
            int value = input.toInt();
            if (value >= 0 && value <= 100 && input.length() > 0 && isDigit(input[0]))
            {
                pwm.set(value);
                printStatus();
            }
        }
    }
}
