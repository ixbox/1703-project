/**
 * PwmOutput - Timer1 PWM出力ライブラリ
 *
 * 契約プログラミングに基づく設計:
 *   Physical - Timer1レジスタ操作
 *   Logical  - PWMチャンネル契約（デューティ比設定能力）
 *
 * Timer1は2チャンネル（OC1A: D9, OC1B: D10）をサポート
 */

#ifndef PWM_OUTPUT_H
#define PWM_OUTPUT_H

#include <Arduino.h>

namespace PwmOutput
{
    // ============================================================
    // Physical Layer - Timer1ハードウェア定義
    // ============================================================

    namespace Hardware
    {
        constexpr uint8_t PIN_OC1A = 9;  // Timer1 Channel A
        constexpr uint8_t PIN_OC1B = 10; // Timer1 Channel B
    }

    // ============================================================
    // Logical Layer - 契約の定義
    // ============================================================

    /**
     * Timer1設定
     * Fast PWM, ICR1をTOPとする可変周波数モード
     */
    struct Timer1Config
    {
        uint16_t frequency_hz;
        uint16_t top; // ICR1値

        static Timer1Config fromFrequency(uint16_t freq_hz)
        {
            uint16_t top = (F_CPU / freq_hz) - 1;
            return {freq_hz, top};
        }

        void apply() const
        {
            // Timer1: Fast PWM, ICR1 as TOP
            // WGM13:0 = 1110 (Fast PWM, TOP=ICR1)
            TCCR1A = (1 << WGM11);
            TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // No prescaler
            ICR1 = top;
        }
    };

    /**
     * PWMチャンネル契約
     * 「デューティ比を設定できる」という能力を提供
     */
    enum class Channel
    {
        A, // OC1A (D9)
        B  // OC1B (D10)
    };

    struct PwmChannel
    {
        const Channel channel;
        const Timer1Config &config;

        void begin()
        {
            uint8_t pin = (channel == Channel::A) ? Hardware::PIN_OC1A : Hardware::PIN_OC1B;
            pinMode(pin, OUTPUT);

            // チャンネル出力を有効化
            if (channel == Channel::A)
            {
                TCCR1A |= (1 << COM1A1);
            }
            else
            {
                TCCR1A |= (1 << COM1B1);
            }

            set(0); // 初期値0%
        }

        void set(uint8_t percent)
        {
            uint16_t duty = ((uint32_t)percent * config.top) / 100;
            if (channel == Channel::A)
            {
                OCR1A = duty;
            }
            else
            {
                OCR1B = duty;
            }
        }

        uint8_t percent() const
        {
            uint16_t duty = (channel == Channel::A) ? OCR1A : OCR1B;
            return (duty * 100) / config.top;
        }
    };

} // namespace PwmOutput

#endif // PWM_OUTPUT_H
