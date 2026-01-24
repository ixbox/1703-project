/**
 * 02-lcd: キャラクタLCD 1602 表示テスト
 *
 * 契約プログラミングに基づく設計:
 *   Physical - ピン定義、ハードウェア初期化
 *   Logical  - LcdDisplay契約
 *   Meaning  - loopには意味だけ
 */

#include <Arduino.h>
#include <LiquidCrystal.h>

// ============================================================
// Physical Layer - ボード固有の定義
// ============================================================

namespace Board
{
    // LCD 1602 (4-bit parallel mode)
    constexpr uint8_t LCD_RS = 2;
    constexpr uint8_t LCD_EN = 3;
    constexpr uint8_t LCD_D4 = 4;
    constexpr uint8_t LCD_D5 = 5;
    constexpr uint8_t LCD_D6 = 6;
    constexpr uint8_t LCD_D7 = 7;
}

namespace LcdConfig
{
    constexpr uint8_t COLS = 16;
    constexpr uint8_t ROWS = 2;
}

// ============================================================
// Logical Layer - 契約の定義
// ============================================================

struct LcdDisplay
{
    LiquidCrystal &lcd;

    void begin()
    {
        lcd.begin(LcdConfig::COLS, LcdConfig::ROWS);
        lcd.clear();
    }

    void showMessage(const char *line1, const char *line2 = nullptr)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(line1);
        if (line2)
        {
            lcd.setCursor(0, 1);
            lcd.print(line2);
        }
    }

    void showUptime()
    {
        unsigned long sec = millis() / 1000;
        unsigned long min = sec / 60;
        sec %= 60;

        lcd.setCursor(0, 1);
        lcd.print(F("Uptime: "));
        if (min < 10)
            lcd.print('0');
        lcd.print(min);
        lcd.print(':');
        if (sec < 10)
            lcd.print('0');
        lcd.print(sec);
        lcd.print(F("   ")); // Clear trailing chars
    }
};

// ============================================================
// Setup - 結線
// ============================================================

LiquidCrystal lcdHw(
    Board::LCD_RS,
    Board::LCD_EN,
    Board::LCD_D4,
    Board::LCD_D5,
    Board::LCD_D6,
    Board::LCD_D7);

LcdDisplay display = {lcdHw};

void setup()
{
    Serial.begin(9600);
    Serial.println(F("=== LCD 1602 Test ==="));
    Serial.println(F("Pins: RS=D2, EN=D3, D4-D7=D4-D7"));

    display.begin();
    display.showMessage("1703 Project", "Initializing...");
    delay(2000);

    Serial.println(F("LCD initialized. Displaying 'Uptime' every second."));
}

// ============================================================
// Loop - 意味だけを書く
// ============================================================

void loop()
{
    display.showUptime();
    delay(1000);
}
