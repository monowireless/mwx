/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "mwx_brd_pal.hpp"
#include "mwx_boards.hpp"

#include "../mwx_twenet.hpp" // if using the_twelite instance.

#include "../sensors/mwx_sns_legacy.hpp"
#include "../sensors/legacy/SHTC3.hpp"
#include "../sensors/legacy/LTR308ALS.hpp"

namespace mwx { inline namespace L1 {
	class BrdPalAmb : public mwx::BrdPal, MWX_APPDEFS_CRTP(BrdPalAmb)
	{
	public:
		static const uint8_t TYPE_ID = mwx::BOARD::PAL_AMB;

		// load common definition for handlers
		#define __MWX_APP_CLASS_NAME BrdPalAmb
		#include "../_mwx_cbs_hpphead.hpp"
		#undef __MWX_APP_CLASS_NAME


	public: // sensor instances
		SnsLTR308ALS sns_LTR308ALS;
		SnsSHTC3 sns_SHTC3;
		
	public:
		// constructor
		BrdPalAmb() {}

		// begin method (if necessary, configure object here)
		void _setup() {
			BrdPal::_hardware_init();
		}

		// begin method (if necessary, start object here)
		void _begin() {
			BrdPal::_begin();

			if (!Wire._has_begun()) {
				Wire.begin();
			}

			sns_LTR308ALS.setup();
			sns_SHTC3.setup();
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

			// Wire object would re-begin, if previous state is active.
			// Wire.begin();
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
