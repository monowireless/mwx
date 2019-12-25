/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "mwx_brd_pal.hpp"

#include "mwx_boards_utils.hpp"
#include "mwx_boards.hpp"

#include "../mwx_periph_dio.hpp"
#include "../mwx_twenet.hpp" // if using the_twelite instance.

namespace mwx { inline namespace L1 {
	class BrdMonoStick : MWX_APPDEFS_CRTP(BrdMonoStick)
	{
	public:
		static const uint8_t PIN_LED = mwx::PIN_DIGITAL::DIO16;  // LED

		static const uint8_t PIN_WDT = mwx::PIN_DIGITAL::DIO9; // WDT (shall tick < 1sec)
		static const uint8_t PIN_WDT_EN = mwx::PIN_DIGITAL::DIO11; // WDT (LO as WDT enabled)

		static const uint8_t PIN_LED_YELLOW = mwx::PIN_DIGITAL::DO1;

	private:
		periph_led_timer _led_red;
		periph_led_timer _led_yellow;

	public:
		static const uint8_t TYPE_ID = mwx::BOARD::MONOSTICK;

		// load common definition for handlers
		#define __MWX_APP_CLASS_NAME BrdMonoStick
		#include "../_mwx_cbs_hpphead.hpp"
		#undef __MWX_APP_CLASS_NAME

	public:
		// constructor
		BrdMonoStick() : _led_red(PIN_LED), _led_yellow(PIN_LED_YELLOW) {}

		// begin method (if necessary, configure object here)
		void _setup() {
			pinMode(PIN_LED, OUTPUT_INIT_HIGH);
			pinMode(PIN_WDT, OUTPUT_INIT_LOW);
			pinMode(PIN_WDT_EN, OUTPUT_INIT_LOW);
			pinMode(PIN_LED_YELLOW, OUTPUT);
		}

		// begin method (if necessary, start object here)
		void _begin() {
			_led_red.begin();
			_led_yellow.begin();
		}

		// set led mode (0: none, 1: blink)
		inline void set_led_yellow(uint8_t mode, uint16_t tick) {
			_led_yellow.setup(mode, tick);
		}

		// set led mode (0: none, 1: blink)
		inline void set_led_red(uint8_t mode, uint16_t tick) {
			_led_red.setup(mode, tick);
		}

	public:
		// TWENET callback handler (mandate)
		void loop() {	
			if (TickTimer.available()) {
				uint32_t ct = millis();
		
				digitalWrite(PIN_WDT, ((ct >> 7) & 0x1) ? HIGH : LOW); // H/L every 128ms

				if (_led_yellow) _led_yellow.tick();
				if (_led_red) _led_red.tick();
			}
		}

		void on_sleep(uint32_t & val) {
			// disable WDT and set those ports HIGH
			digitalWrite(PIN_WDT, HIGH);
			digitalWrite(PIN_WDT_EN, HIGH);
			
			// disable LEDs
			digitalWrite(PIN_LED, HIGH);
			pinMode(PIN_LED_YELLOW, DISABLE_OUTPUT);
		}

		void warmboot(uint32_t & val) { }
		void wakeup(uint32_t & val) {
			// DO1 needs to set output again.
			pinMode(PIN_LED_YELLOW, OUTPUT);
		}

		void on_create(uint32_t& val) { _setup();  }
		void on_begin(uint32_t& val) { _begin(); }
		void on_message(uint32_t& val) { }

	public: // never called the following as hardware class, but define it!
		void network_event(mwx::packet_ev_nwk& ev) {}
		void receive(mwx::packet_rx& rx) {
			_led_red.begin(periph_led_timer::ON_RX);
			_led_yellow.begin(periph_led_timer::ON_RX);
		}
		void transmit_complete(mwx::packet_ev_tx& ev) {
			_led_red.begin(periph_led_timer::ON_TX_COMP);
			_led_yellow.begin(periph_led_timer::ON_TX_COMP);
		}
	};
}}
