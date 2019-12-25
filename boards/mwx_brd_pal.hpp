/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "_tweltite.hpp"
#include "mwx_boards_utils.hpp"

namespace mwx { inline namespace L1 {
    // common procedure for PAL board
    class BrdPal {
    public:
        static const uint8_t PIN_BTN = 12; // button (as SET)
		static const uint8_t PIN_LED = 5;  // LED
		static const uint8_t PIN_WDT = 13; // WDT (shall tick every 60sec)

		static const uint8_t PIN_D1 = 1; // DIP SW1
		static const uint8_t PIN_D2 = 2; // DIP SW2
		static const uint8_t PIN_D3 = 3; // DIP SW3
		static const uint8_t PIN_D4 = 4; // DIP SW4

		static const uint8_t PIN_SNS_EN = 16;
		static const uint8_t PIN_SNS_INT = 17;

	protected:
		uint8_t _dipsw_bm; // 0 as LOW, 1 as HIGH
		periph_led_timer _led; // LED Timer handling

    public:
		BrdPal() : _dipsw_bm(0), _led(PIN_LED) {}

		void setup() {
			_dipsw_bm = 0;
		}

		// set led mode (0: none, 1: blink)
		inline void set_led(uint8_t mode, uint16_t tick) {
			_led.setup(mode, tick);
		}

		// note: change mode to LED_TIMER.
		inline void led_one_shot(uint16_t tick) {
			_led.setup(periph_led_timer::ONESHOT, tick);
			_led.begin(periph_led_timer::ONESHOT);
		}

	public:
		// DIP SW (inverting..., LOW as set(1), HIGH as unset(0))
		inline uint8_t get_D1() { return (_dipsw_bm & (1 << 0)) ? 0 : 1; }
		inline uint8_t get_D2() { return (_dipsw_bm & (1 << 1)) ? 0 : 1; }
		inline uint8_t get_D3() { return (_dipsw_bm & (1 << 2)) ? 0 : 1; }
		inline uint8_t get_D4() { return (_dipsw_bm & (1 << 3)) ? 0 : 1; }
		inline uint8_t get_DIPSW_BM() { return ~_dipsw_bm & 0x0F; }

	protected:

		void _begin() {
			// LED
			_led.begin();
		}

		void _wakeup() {
			// WDT
			pinMode(PIN_WDT, OUTPUT_INIT_LOW); // just in case, if some code set pinMode to different mode.
			
			// LED
			if (_led) _led.begin();
		}

		void _hardware_init() {
			pinMode(PIN_BTN, INPUT_PULLUP);
			pinMode(PIN_LED, OUTPUT_INIT_HIGH);
			pinMode(PIN_WDT, OUTPUT_INIT_HIGH);
			
			pinMode(PIN_D1, INPUT_PULLUP);
			pinMode(PIN_D2, INPUT_PULLUP);
			pinMode(PIN_D3, INPUT_PULLUP);
			pinMode(PIN_D4, INPUT_PULLUP);

			// read dip sw status, when finished, disable pullup for pins at LOW.
			if (digitalRead(PIN_D1) == HIGH) _dipsw_bm |= (1 << 0); else pinMode(PIN_D1, INPUT);
			if (digitalRead(PIN_D2) == HIGH) _dipsw_bm |= (1 << 1); else pinMode(PIN_D2, INPUT);
			if (digitalRead(PIN_D3) == HIGH) _dipsw_bm |= (1 << 2); else pinMode(PIN_D3, INPUT);
			if (digitalRead(PIN_D4) == HIGH) _dipsw_bm |= (1 << 3); else pinMode(PIN_D4, INPUT);
		}

		void _loop() {
			if (TickTimer.available()) {
				uint32_t tick = millis();

				// WDT
				digitalWrite(PIN_WDT, (tick & 1) ? HIGH : LOW);

				// LED
				if(_led) _led.tick();
			}
		}

		void _on_sleep() {
			// set high for save sleeping current.
			pinMode(PIN_WDT, OUTPUT_INIT_HIGH); // just in case, if some code set pinMode to different mode.

			// LED
			if (_led) digitalWrite(PIN_LED, HIGH);
		}

		void _receive() {
			// LED
			if(_led) _led.begin(periph_led_timer::ON_RX);
		}

		void _transmit_complete() {
			// LED
			if(_led) _led.begin(periph_led_timer::ON_TX_COMP);
		}
    };

}}
