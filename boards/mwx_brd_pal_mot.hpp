/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "mwx_brd_pal.hpp"
#include "mwx_boards.hpp"

#include "../mwx_twenet.hpp" // if using the_twelite instance.

#include "../sensors/mwx_sns_MC3630.hpp"

namespace mwx { inline namespace L1 {
	class BrdPalMot : public mwx::BrdPal, MWX_APPDEFS_CRTP(BrdPalMot)
	{
	public:
		static const uint8_t TYPE_ID = mwx::BOARD::PAL_AMB;

		// load common definition for handlers
		#define __MWX_APP_CLASS_NAME BrdPalMot
		#include "../_mwx_cbs_hpphead.hpp"
		#undef __MWX_APP_CLASS_NAME


	public: // sensor instances
		SnsMC3630 sns_MC3630;
		
	public:
		// constructor
		BrdPalMot() {}

		// begin method (if necessary, configure object here)
		void _setup() {
			BrdPal::_hardware_init();
			sns_MC3630.setup();
		}

		// begin method (if necessary, start object here)
		void _begin() {
			BrdPal::_begin();

			pinMode(PIN_SNS_INT, INPUT_PULLUP);
			attachIntDio(PIN_SNS_INT, FALLING);
		}

	public:
		// TWENET callback handler (mandate)
		void loop() {
			BrdPal::_loop();
		}

		void on_sleep(uint32_t & val) {
			BrdPal::_on_sleep();
		}

		void warmboot(uint32_t & val) {}

		void wakeup(uint32_t & val) {
			BrdPal::_wakeup();

			pinMode(PIN_SNS_INT, INPUT_PULLUP);
			attachIntDio(PIN_SNS_INT, FALLING);

			sns_MC3630.wakeup();
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
