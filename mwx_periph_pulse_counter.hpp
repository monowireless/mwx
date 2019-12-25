/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdint>
#include <cstring>

#include <jendefs.h>
#include <AppHardwareApi.h>

#include "mwx_debug.h"
#include "mwx_hardware.hpp"
#include "mwx_utils.hpp"

#include "mwx_periph_dio.hpp"

namespace mwx { inline namespace L1 {

    /**
     * Pulse Counter handling
     * - limited support only for DIO8 (assuming used with PAL J2/D8)
     */
	class periph_pulse_counter {
        uint8_t _u8_device;
        uint8_t _b_started;

        uint16_t _u16_refct;

	private:
		periph_pulse_counter(const periph_pulse_counter&) = delete;
		periph_pulse_counter& operator =(const periph_pulse_counter&) = delete;
		inline bool _begun() { return _b_started;  }

    public:
        static const uint8_t DEVICE_PC1 = E_AHI_PC_1; // default is PC1
        static const uint8_t DEVICE_PC0 = E_AHI_PC_0;
        
    public:
        periph_pulse_counter(uint8_t u8dev) : _u8_device(u8dev), _b_started(0), _u16_refct(0) {}

        void setup() {
        } // nothing so far.

        void begin(uint16_t refct = 0, 
                   E_PIN_INT_MODE edge = PIN_INT_MODE::FALLING,
                   uint8_t debounce = 0) {

            
            // configure the counter
            bAHI_PulseCounterConfigure(
                _u8_device,
                edge == PIN_INT_MODE::FALLING ? 1 : 0,      // 0:RISE, 1:FALL EDGE
                debounce,                       // Debounce 0:off, 1:2samples, 2:4samples, 3:8samples
                FALSE,                          // Combined Counter
                refct > 0 ? TRUE : FALSE);      // Interrupt

            // reference counter
            bAHI_SetPulseCounterRef(
                _u8_device,
                refct); // set reference counter in advance

            // start counter
            bAHI_StartPulseCounter(_u8_device); // start it

            _u16_refct = refct;
            _b_started = true;
        }

        void end() {
            bAHI_StopPulseCounter(_u8_device);
            _b_started = false;
        }

        inline bool available() {
            if (_b_started) {
                if (_u16_refct > 0) { // reaches ref value or not (interrupt base)
                    bool ret = ((_b_started & 0x80) != 0);
                    _b_started &= 0x7F;

                    return ret;
                } else { // has some value.
                    return _read_wo_clear() > 0 ? true : false;
                }
            } else
                return false;
        }

        uint16_t read() {
            uint16_t val;
            bAHI_Read16BitCounter(_u8_device, &val); // read
            bAHI_Clear16BitPulseCounter(_u8_device); // clear it!
            return val;
        }

    public:
        inline void _int_pc(uint32_t u32bitmap) {
            if (_u8_device == DEVICE_PC1 && (1UL << 23)) { _b_started |= 0x80; return; }
            if (_u8_device == DEVICE_PC0 && (1UL << 22)) { _b_started |= 0x80; return; }
        }

        inline uint16_t _read_wo_clear() {
            uint16_t val;
            bAHI_Read16BitCounter(_u8_device, &val);
            return val;
        }

#if 0 // this does not work when interrupt enabled.
        inline bool _reach_refcount() {
            if (_b_started && _u16_refct > 0) { // reaches ref value or not (interrupt base)
                if (u32AHI_PulseCounterStatus() & (1UL << ((_u8_device == DEVICE_PC1 ? 23 : 22)))) {
                    return true;
                }
            }
            return false;
        }
#endif

        void _on_sleep() {
            // nothing, pulse counter keeps running during sleep.
        }

        void _on_wakeup() {
            // nothing, you can read the count value as well.
            if (_b_started) {
                if (_read_wo_clear() >= _u16_refct) _b_started |= 0x80;
            }
        }
    };

}}