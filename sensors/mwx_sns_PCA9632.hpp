/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "mwx_sensors.hpp"

namespace mwx {
    inline namespace L1 {
        
        class SnsPCA9632 {
            uint8_t _i2c_addr; //! device I2C address

        public:
            /**
             * @brief Construct a new SnsPCA9632 object
             *        (note: it's never called if declared in global scope)
             * @param i2c_addr device address over I2C
             */
            SnsPCA9632(uint8_t i2c_addr = DEFAULT_I2C_ADDRESS) { setup(i2c_addr); }

            /**
             * @brief init value (doing same when constuctor is called)
             * 
             * @param i2c_addr device address over I2C
             */
            void setup(uint8_t i2c_addr = DEFAULT_I2C_ADDRESS) {
                _i2c_addr = i2c_addr;
            }

            /**
             * @brief set registers as defaults.
             * 
             * @return true success
             * @return false fail to communicate with the device. 
             */
            bool reset();

            /**
             * @brief set MODE2 register
             *        0x35(default value) : Reverse Hi/Lo, Blink, Totempole
             * 
             * @param u8var register variable
             * @return true success
             * @return false fail to communicate with the device.
             */
            bool set_mode2(uint8_t u8var = 0x35);

            /**
             * @brief set active or sleep mode
             * 
             * @param b_pow_on true: active, false: sleep
             * 
             * @return true success
             * @return false fail to communicate with the device.
             */
            bool set_power_mode(bool b_pow_on);

            /**
             * @brief set blinking duty (group PWM register)
             * 
             * @param u8duty 0..255 (u8duty/255), 255: turning on for almost full time (99.6%)
             * @return true success
             * @return false fail to communicate with the device.
             */
            bool set_blink_duty(uint8_t u8duty);

            /**
             * @brief set blinking cycle (group FREQ register)
             * 
             * @param u8var cycle time, (u8var+1)/24 [sec]
             * @return true success
             * @return false fail to communicate with the device.
             */
            bool set_blink_cycle(uint8_t u8var);

            // set blinking duty period in u16ms[ms].
            bool set_blink_cycle_ms(uint16_t u16ms) {
                int32_t var = (3146*u16ms+65000)/131072 - 1; // approx calc of `Tms = (u8var + 1)*1000/24'
                return set_blink_cycle(var < 0 ? 0 : (var > 255 ? 255 : var));
            }

            /**
             * @brief set PWM duty for each port (brightness)
             *  
             * @param port port number to change (0..3)
             * @param duty duty paramter (0..255, set 255 to light almost fully)
             * @return true success
             * @return false fail to communicate with the device. 
             */
            bool set_led_duty(uint8_t port, uint8_t duty);

            /**
             * @brief set brightness for all ports
             * 
             * @param u8led1 duty paramter for LED1 (0..255, set 255 to light almost fully)
             * @param u8led2 duty paramter for LED2 (0..255, set 255 to light almost fully) 
             * @param u8led3 duty paramter for LED3 (0..255, set 255 to light almost fully)
             * @param u8led4 duty paramter for LED4 (0..255, set 255 to light almost fully)
             * @return true success
             * @return false fail to communicate with the device. 
             */
            bool set_led_duty_all(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);

            /**
             * @brief set LED lighting mode for all nodes (OFF, ON, PWM, BLINK)
             *        the possible parameter values are listed as below:
             *          - LED_OFF(0) -> turn off
             *          - LED_ON(1)  -> fully on (max brightness)
             *          - LED_PWM(2) -> PWM control
             *          - LED_BLINK(3) -> blinking (GROUP PWM)
             *                            note: the each PWM duty settings is reflected.
             *          - LED_NOP(0xff) -> no-operation
             * 
             * @param u8led1 control LED1
             * @param u8led2 control LED2
             * @param u8led3 control LED3
             * @param u8led4 control LED4
             * @return true success
             * @return false fail to communicate with the device.  
             */
            bool set_led_status(uint8_t u8led1, uint8_t u8led2, uint8_t u8led3, uint8_t u8led4);

            /**
             * @brief probe I2C device
             * 
             * @return true success (found the device with _i2c_addr)
             * @return false fail to communicate with the device.  
             */
            bool probe() {
                return Wire.probe(_i2c_addr);
            }

            /**
             * @brief showing registers to Serial.
             *        for debugging purpose.
             */
            void show_registers();

        public:
            static const uint8_t DEFAULT_I2C_ADDRESS = 0x62;
            static const uint8_t PCA9632_MODE1 = 0x00;
            static const uint8_t PCA9632_MODE2 = 0x01;
            static const uint8_t PCA9632_PWM0 = 0x02;
            static const uint8_t PCA9632_PWM1 = 0x03;
            static const uint8_t PCA9632_PWM2 = 0x04;
            static const uint8_t PCA9632_PWM3 = 0x05;
            static const uint8_t PCA9632_GRPPWM = 0x06;
            static const uint8_t PCA9632_GRPFREQ = 0x07;
            static const uint8_t PCA9632_LEDOUT = 0x08;
            static const uint8_t PCA9632_SUBADDR1 = 0x09;
            static const uint8_t PCA9632_SUBADDR2 = 0x0A;
            static const uint8_t PCA9632_SUBADDR3 = 0x0B;
            static const uint8_t PCA9632_ALLCALLADR = 0x0C;

            static const uint8_t LED_OFF = 0;
            static const uint8_t LED_ON = 1;
            static const uint8_t LED_PWM = 2;
            static const uint8_t LED_BLINK = 3;
            static const uint8_t LED_NOP = 0xFF;
            
            static const uint8_t LED1 = 0;
            static const uint8_t LED2 = 1;
            static const uint8_t LED3 = 2;
            static const uint8_t LED4 = 3;
        };
    }
}