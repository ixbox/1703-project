/**
 * MotorDriver - BTS7960モータードライバー制御ライブラリ
 *
 * 契約プログラミングに基づく設計:
 *   Physical - ピン定義、BTS7960ハードウェア
 *   Logical  - モーター契約（速度設定能力、停止能力）
 *
 * BTS7960制御方式:
 *   正転: RPWM=duty, LPWM=0
 *   逆転: RPWM=0, LPWM=duty
 *   停止: RPWM=0, LPWM=0
 *   ブレーキ: RPWM=100%, LPWM=100%
 */

#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>
#include <PwmOutput.h>

namespace MotorDriver
{
    // ============================================================
    // Logical Layer - 契約の定義
    // ============================================================

    /**
     * BTS7960モータードライバー契約
     * 「速度を設定できる」「停止できる」という能力を提供
     */
    struct Bts7960
    {
        PwmOutput::PwmChannel &rpwm; // 正転PWM
        PwmOutput::PwmChannel &lpwm; // 逆転PWM
        const uint8_t r_en;          // 右イネーブルピン
        const uint8_t l_en;          // 左イネーブルピン

        int8_t current_speed; // -100 to +100

        void begin()
        {
            pinMode(r_en, OUTPUT);
            pinMode(l_en, OUTPUT);

            rpwm.begin();
            lpwm.begin();

            disable();
            current_speed = 0;
        }

        void enable()
        {
            digitalWrite(r_en, HIGH);
            digitalWrite(l_en, HIGH);
        }

        void disable()
        {
            digitalWrite(r_en, LOW);
            digitalWrite(l_en, LOW);
        }

        /**
         * 速度設定
         * @param speed -100〜+100 (負:逆転, 0:停止, 正:正転)
         */
        void setSpeed(int8_t speed)
        {
            speed = constrain(speed, -100, 100);
            current_speed = speed;

            if (speed > 0)
            {
                // 正転
                lpwm.set(0);
                rpwm.set(speed);
            }
            else if (speed < 0)
            {
                // 逆転
                rpwm.set(0);
                lpwm.set(-speed);
            }
            else
            {
                // 停止（コースト）
                rpwm.set(0);
                lpwm.set(0);
            }
        }

        void stop()
        {
            setSpeed(0);
        }

        /**
         * ブレーキ（ショートブレーキ）
         * 両方のPWMをHIGHにしてモーターをショート
         *
         * NOTE: ウォームギア駆動ではセルフロック特性があるため、
         *       ショートブレーキは不要。stop()で十分。
         *       実運用時は要検討。
         */
        void brake()
        {
            rpwm.set(100);
            lpwm.set(100);
            current_speed = 0;
        }

        int8_t speed() const
        {
            return current_speed;
        }

        bool isEnabled() const
        {
            return digitalRead(r_en) == HIGH && digitalRead(l_en) == HIGH;
        }
    };

} // namespace MotorDriver

#endif // MOTOR_DRIVER_H
