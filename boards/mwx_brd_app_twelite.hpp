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
	class BrdAppTwelite : MWX_APPDEFS_CRTP(BrdAppTwelite)
	{
	public:
		static const uint8_t PIN_DI1 = mwx::PIN_DIGITAL::DIO12;
        static const uint8_t PIN_DI2 = mwx::PIN_DIGITAL::DIO13;
        static const uint8_t PIN_DI3 = mwx::PIN_DIGITAL::DIO11;
        static const uint8_t PIN_DI4 = mwx::PIN_DIGITAL::DIO16;

		static const uint8_t PIN_DO1 = mwx::PIN_DIGITAL::DIO18;
        static const uint8_t PIN_DO2 = mwx::PIN_DIGITAL::DIO19;
        static const uint8_t PIN_DO3 = mwx::PIN_DIGITAL::DIO4;
        static const uint8_t PIN_DO4 = mwx::PIN_DIGITAL::DIO9;

		static const uint8_t PIN_M1 = mwx::PIN_DIGITAL::DIO10;
        static const uint8_t PIN_M2 = mwx::PIN_DIGITAL::DIO2;
        static const uint8_t PIN_M3 = mwx::PIN_DIGITAL::DIO3;
        static const uint8_t PIN_BPS = mwx::PIN_DIGITAL::DIO17;

        static const uint8_t PIN_AI1 = mwx::PIN_ANALOGUE::A1;
        static const uint8_t PIN_AI2 = mwx::PIN_ANALOGUE::A3;
        static const uint8_t PIN_AI3 = mwx::PIN_ANALOGUE::A2;
        static const uint8_t PIN_AI4 = mwx::PIN_ANALOGUE::A4;

	private:
		uint8_t _dipsw_bm;

	public:
		static const uint8_t TYPE_ID = mwx::BOARD::BRD_APPTWELITE;

		// load common definition for handlers
		#define __MWX_APP_CLASS_NAME BrdAppTwelite
		#include "../_mwx_cbs_hpphead.hpp"
		#undef __MWX_APP_CLASS_NAME

	public:
		// DIP SW (inverting..., LOW as set(1), HIGH as unset(0))
		inline uint8_t get_M1()  { return (_dipsw_bm & 1) ? 0 : 1; }
		inline uint8_t get_M2()  { return (_dipsw_bm & 2) ? 0 : 1; }
		inline uint8_t get_M3()  { return (_dipsw_bm & 4) ? 0 : 1; }
		inline uint8_t get_BPS() { return (_dipsw_bm & 8) ? 0 : 1; }
		inline uint8_t get_DIPSW_BM() { return  ~_dipsw_bm & 0x0F; }

		// digital ports (HIGH as 1, LOW as 0)
		//   u32AHI_DioReadInput() returns bitmap, setting 1 as high.
		inline uint8_t get_DI_BM() {
			return uint8_t(mwx::collect_bits(u32AHI_DioReadInput(), PIN_DI4, PIN_DI3, PIN_DI2, PIN_DI1));
		} 

	public:
		// constructor
		BrdAppTwelite() {}

		// begin method (if necessary, configure object here)
		void _setup() {
			pinMode(PIN_DI1, INPUT_PULLUP);
			pinMode(PIN_DI2, INPUT_PULLUP);
			pinMode(PIN_DI3, INPUT_PULLUP);
			pinMode(PIN_DI4, INPUT_PULLUP);
			pinMode(PIN_DO1, OUTPUT_INIT_HIGH);
			pinMode(PIN_DO2, OUTPUT_INIT_HIGH);
			pinMode(PIN_DO3, OUTPUT_INIT_HIGH);
			pinMode(PIN_DO4, OUTPUT_INIT_HIGH);
            pinMode(PIN_M1, INPUT_PULLUP);
			pinMode(PIN_M2, INPUT_PULLUP);
			pinMode(PIN_M3, INPUT_PULLUP);
			pinMode(PIN_BPS, INPUT_PULLUP);

			_dipsw_bm = 0;
			if (digitalRead(PIN_M1) == HIGH)  _dipsw_bm |= (1 << 0); else pinMode(PIN_M1, INPUT); // if low, unset pullup.
			if (digitalRead(PIN_M2) == HIGH)  _dipsw_bm |= (1 << 1); else pinMode(PIN_M2, INPUT);
			if (digitalRead(PIN_M3) == HIGH)  _dipsw_bm |= (1 << 2); else pinMode(PIN_M3, INPUT);
			if (digitalRead(PIN_BPS) == HIGH) _dipsw_bm |= (1 << 3); else pinMode(PIN_BPS, INPUT);

            pinMode(mwx::PIN_DIGITAL::DIO0, INPUT); // ADC3
            pinMode(mwx::PIN_DIGITAL::DIO1, INPUT); // ADC4
		}

		// begin method (if necessary, start object here)
		void _begin() {
            // relocate PWM pins (DO1/DO2)
            vAHI_TimerSetLocation(E_AHI_TIMER_1, TRUE, TRUE); // DIO5, DO1, DO2, DIO8

            Timer1.begin(1000, false, true);
            Timer1.change_duty(1024);
            Timer2.begin(1000, false, true);
            Timer2.change_duty(1024);
            Timer3.begin(1000, false, true);
            Timer3.change_duty(1024);
            Timer4.begin(1000, false, true);
            Timer4.change_duty(1024);
		}

	public:
		// TWENET callback handler (mandate)
		void loop() {	
			if (TickTimer.available()) {
				uint32_t ct = millis(); (void)ct;
			}
		}

		void on_sleep(uint32_t & val) {
		}

		void warmboot(uint32_t & val) { }
		void wakeup(uint32_t & val) {
		}

		void on_create(uint32_t& val) { _setup();  }
		void on_begin(uint32_t& val) { _begin(); }
		void on_message(uint32_t& val) { }

	public: // never called the following as hardware class, but define it!
		void network_event(mwx::packet_ev_nwk& ev) {}
		void receive(mwx::packet_rx& rx) {
		}
		void transmit_complete(mwx::packet_ev_tx& ev) {
		}
	};
}}
