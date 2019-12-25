/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "mwx_brd_pal.hpp"
#include "mwx_boards.hpp"

#include "../mwx_twenet.hpp" // if using the_twelite instance.

namespace mwx { inline namespace L1 {
	class BrdPalMag : public mwx::BrdPal, MWX_APPDEFS_CRTP(BrdPalMag)
	{
	public:
		static const uint8_t TYPE_ID = mwx::BOARD::PAL_MAG;

		// load common definition for handlers
		#define __MWX_APP_CLASS_NAME BrdPalMag
		#include "../_mwx_cbs_hpphead.hpp"
		#undef __MWX_APP_CLASS_NAME

	public:
        static const uint8_t PIN_SNS_NORTH = 16;
        static const uint8_t PIN_SNS_OUT1 = 16;
        static const uint8_t PIN_SNS_SOUTH = 17;
        static const uint8_t PIN_SNS_OUT2 = 17;

	public:
		// constructor
		BrdPalMag() {}

		// begin method (if necessary, configure object here)
		void _setup() {
			BrdPal::_hardware_init();

            // set sensor pin as input
            pinMode(PIN_SNS_OUT1, PIN_MODE::INPUT);
            pinMode(PIN_SNS_OUT2, PIN_MODE::INPUT);
		}

		// begin method (if necessary, start object here)
		void _begin() {
			BrdPal::_begin();
		}

	public:
		// TWENET callback handler (mandate)
		void loop() {
			BrdPal::_loop();
		}

		void on_sleep(uint32_t & val) {
			_led._on_sleep();
			BrdPal::_on_sleep();
		}

		void warmboot(uint32_t & val) {}

		void wakeup(uint32_t & val) {
			_led._on_wakeup();
			BrdPal::_wakeup();
		}

		void on_create(uint32_t& val) { _setup();  }
		void on_begin(uint32_t& val) { _begin(); }
		void on_message(uint32_t& val) { }

	public: // never called the following as hardware class, but define it!
		void network_event(mwx::packet_ev_nwk& pEvNwk) {}
		void receive(mwx::packet_rx& rx) {
			BrdPal::_receive();
		}
		void transmit_complete(mwx::packet_ev_tx& pEvTx) {
			BrdPal::_transmit_complete();
		}
	};
}}
