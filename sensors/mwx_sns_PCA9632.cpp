/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include <TWELITE>
#include "mwx_sns_PCA9632.hpp"

// NOTE: set DIO16 as LOW to turn on LED (FET s/w on) if using `Notice PAL'.


bool SnsPCA9632::reset() {
    if (auto&& x = Wire.get_writer(_i2c_addr)) {
        x << 0x80; // incremental address.
        x << 0x81; // MODE1
        x << 0x35; // MODE2
        x << 127; // PWM1
        x << 127; // PWM2
        x << 127; // PWM3
        x << 127; // PWM4
        x << 127; // GRPPWM
        x << 11; // GRPFREQ
        x << 0x00; // 0b01010101; // LED OUT
    } else return false;
    
    return true;
}


bool SnsPCA9632::set_mode2(uint8_t u8var) {
    if (auto&& x = Wire.get_writer(_i2c_addr)) {
        x << PCA9632_MODE2; x << u8var;
    } else return false;

    return true;
}


bool SnsPCA9632::set_power_mode(bool b_pow_on) {
    uint8_t data = b_pow_on ? 0x81 : 0x91;

    if (auto&& x = Wire.get_writer(_i2c_addr)) {
        x << PCA9632_MODE1;
        x << data;
    } else return false;

    return true;
}


bool SnsPCA9632::set_blink_duty(uint8_t u8duty) {
    if (auto&& x = Wire.get_writer(_i2c_addr)) {
        x << PCA9632_GRPPWM;
        x << u8duty;
    } else return false;
    
    return true;
}


bool SnsPCA9632::set_blink_cycle(uint8_t u8var) {
    if (auto&& x = Wire.get_writer(_i2c_addr)) {
        x << PCA9632_GRPFREQ;
        x << u8var;
    } else return false;

    return true;
}


bool SnsPCA9632::set_led_duty(uint8_t port, uint8_t duty) {
    if (auto&& x = Wire.get_writer(_i2c_addr)) {
        x << uint8_t(PCA9632_PWM0 + port);
        x << duty;
    } else return false;

    return true;
}



bool SnsPCA9632::set_led_duty_all(uint8_t u8led1, uint8_t u8led2, uint8_t u8led3, uint8_t u8led4) {
    if (auto&& x = Wire.get_writer(_i2c_addr)) {
        x << uint8_t(PCA9632_PWM0 + 0x80);
        x << u8led1;
        x << u8led2;
        x << u8led3;
        x << u8led4;
    } else return false;
    
    return true;
}


bool SnsPCA9632::set_led_status(uint8_t u8led1, uint8_t u8led2, uint8_t u8led3, uint8_t u8led4) {
    if (auto&& x = Wire.get_writer(_i2c_addr)) {
        x << PCA9632_LEDOUT;
    } else return false;

    uint8_t data;
    if (auto && x = Wire.get_reader(_i2c_addr, 1)) {
        x >> data;
    } else return false;

    u8led1 = (u8led1 == LED_NOP) ? (data & 0x03) >> 0 : (u8led1 & 0x03); u8led1 <<= 0; // LED1 (b0b1)
    u8led2 = (u8led2 == LED_NOP) ? (data & 0x0C) >> 2 : (u8led2 & 0x03); u8led2 <<= 2; // LED2 (b2b3)
    u8led3 = (u8led3 == LED_NOP) ? (data & 0x30) >> 4 : (u8led3 & 0x03); u8led3 <<= 4; // LED3 (b4b5)
    u8led4 = (u8led4 == LED_NOP) ? (data & 0xC0) >> 6 : (u8led4 & 0x03); u8led4 <<= 6; // LED4 (b6b7)

    data = u8led1 | u8led2 | u8led3 | u8led4;

    if (auto&& x = Wire.get_writer(_i2c_addr)) {
        x << (0x80 + PCA9632_LEDOUT);
        x << data;
    } else return false;

    set_power_mode(!data ? false : true);

    return true;
}


void SnsPCA9632::show_registers() {
    Serial << "{";
    
    // set start address with incremental reading.
    if (auto&& x = Wire.get_writer(_i2c_addr)) { x << (0x80); }

    // read burst
    const uint8_t MAXADDR = 0x08;
    uint8_t d;
    if (auto&& x = Wire.get_reader(_i2c_addr, MAXADDR + 1)) {
        for(int i = 0; i <= MAXADDR; i++) {
            x >> d;
            if (i > 0) Serial << ',';
            Serial << format("%02X", d);
        }
    }

    Serial << "}";
}