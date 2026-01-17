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

namespace Board {
    constexpr uint8_t PWM_PIN = 9;  // Timer1 PWM
}

// ============================================================
// Logical Layer - 契約の定義
// ============================================================

struct PwmOutput {
    const uint8_t pin;
    uint8_t duty;

    void begin() {
        pinMode(pin, OUTPUT);
        apply();
    }

    void set(uint8_t value) {
        duty = value;
        apply();
    }

    void apply() {
        analogWrite(pin, duty);
    }

    uint8_t percent() const {
        return (duty * 100) / 255;
    }
};

struct SerialCommand {
    bool hasCommand() {
        return Serial.available() > 0;
    }

    String take() {
        String input = Serial.readStringUntil('\n');
        input.trim();
        return input;
    }
};

// ============================================================
// Meaning Layer - 意味の定義
// ============================================================

namespace Command {
    constexpr char INCREASE = '+';
    constexpr char DECREASE = '-';
    constexpr char STATUS   = '?';
}

// ============================================================
// Setup - 結線
// ============================================================

PwmOutput pwm = { Board::PWM_PIN, 128 };
SerialCommand cmd;

void printStatus() {
    Serial.print(F("Duty: "));
    Serial.print(pwm.duty);
    Serial.print(F(" ("));
    Serial.print(pwm.percent());
    Serial.println(F("%)"));
}

void printHelp() {
    Serial.println(F("=== PWM Basic Test ==="));
    Serial.println(F("Commands:"));
    Serial.println(F("  0-255 : Set duty cycle"));
    Serial.println(F("  +     : Increase by 10"));
    Serial.println(F("  -     : Decrease by 10"));
    Serial.println(F("  ?     : Show current status"));
    Serial.println();
}

void setup() {
    Serial.begin(9600);
    pwm.begin();

    printHelp();
    printStatus();
}

// ============================================================
// Loop - 意味だけを書く
// ============================================================

void loop() {
    if (cmd.hasCommand()) {
        String input = cmd.take();

        if (input[0] == Command::INCREASE) {
            pwm.set(min(255, pwm.duty + 10));
            printStatus();
        }
        else if (input[0] == Command::DECREASE) {
            pwm.set(max(0, pwm.duty - 10));
            printStatus();
        }
        else if (input[0] == Command::STATUS) {
            printStatus();
        }
        else {
            int value = input.toInt();
            if (value >= 0 && value <= 255 && input.length() > 0 && isDigit(input[0])) {
                pwm.set(value);
                printStatus();
            }
        }
    }
}
