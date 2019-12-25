/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../mwx_debug.h"
#include <ToCoNet.h>

#include "../mwx_utils_smplque.hpp"

#include "mwx_packet_ev.hpp"
#include "mwx_packet_tx.hpp"
#include "mwx_packet_rx.hpp"

namespace mwx { inline namespace L1 {
	class twenet;
	
	namespace NETWORK {
		static const uint8_t NONE = 0;
		static const uint8_t SIMPLE = 1;
	};

	class twenet_tx_cb_management {
		uint32_t _bm_l_active;
		uint32_t _bm_l_result;
		uint32_t _bm_h_active;
		uint32_t _bm_h_result;

		inline void _set_on_request(int bit, uint32_t& bm_active, uint32_t& bm_result) {
			uint32_t m = (1UL << bit);
			//uint32_t m_inv = ~m;

			bm_active |= m;
		}
		inline void _set_on_event(int bit, bool stat, uint32_t& bm_active, uint32_t& bm_result) {
			uint32_t m = (1UL << bit);
			uint32_t m_inv = ~m;
			bm_active &= m_inv; // clear bit
			bm_result &= m_inv; // clear bit
			if (stat) {
				bm_result |= m;
			}
		}

	public:
		twenet_tx_cb_management() : _bm_l_active(0), _bm_h_active(0) {}

		void _tx_request(uint8_t cbid) {
			if (cbid < 32) {
				_set_on_request(cbid, _bm_l_active, _bm_l_result);
			}
			else if (cbid < 64) {
				_set_on_request(cbid - 32, _bm_h_active, _bm_h_result);
			}
			else {
				; // ignore...
			}
		}
		void _tx_event(uint8_t cbid, uint8_t stat) {
			if (cbid < 32) {
				_set_on_event(cbid, stat, _bm_l_active, _bm_l_result);
			}
			else if (cbid < 64) {
				_set_on_event(cbid - 32, stat, _bm_h_active, _bm_h_result);
			}
			else {
				; // ignore the event.
			}
		}

		bool is_complete(uint8_t cbid) {
			bool b_comp = true;
			if (cbid < 32) {
				b_comp = !((1UL << cbid) & _bm_l_active);
			}
			else if (cbid < 64) {
				b_comp = !((1UL << (cbid -32)) & _bm_h_active);
			}
			return b_comp;
		}

		bool is_success(uint8_t cbid) {
			bool b_succ = true;
			if (cbid < 32) {
				b_succ = !((1UL << cbid) & _bm_l_result);
			}
			else if (cbid < 64) {
				b_succ = !((1UL << (cbid - 32)) & _bm_h_result);
			}
			return b_succ;
		}
	};
	
	// note: this queue is not safe, saving discarded queue pointer in TWENET.
	//       however, the pointer may not be destroyed until coming several packets.
	class twenet_rx_packet_queue {
		// the tsRxDataApp* from TWENET is local var, so make a copy of this struct for further reference.
		using pair_rx = std::pair<mwx::packet_rx, tsRxDataApp>;
		mwx::smplque<pair_rx, mwx::alloc_local<pair_rx, 2>> _que; // only two entries queue!

	public:
		void setup() {}
		bool available() { return !_que.empty(); }

		mwx::packet_rx& read() {
			_que.front().first._set_psRxDataApp(&_que.front().second);
			return _que.pop_front().first;
		}

		void _push(mwx::packet_rx& rx) {
			if(_que.is_full()) _que.clear(); // if full, clean up
			_que.push(std::make_pair(rx, *(rx.get_psRxDataApp())));
		}
	};

	// collection of some MAC service.
	class twenet_mac {
	public:
		twenet_mac() {}

		void setup() {
		}

		void begin() {
		}

		// transmit a packet
		// note: if u8CbId is set 0xFF, 0..31 is used automatically, where user 
		//       specified u8CbId is NOT considered (may conflict).
		MWX_APIRET transmit(mwx::packet_tx& pkt) {
			static uint8_t u8seq;
			++u8seq;

			uint8_t u8cbid = pkt.get_psTxDataApp()->u8CbId;

			// set call back ID (CbId)
			if (u8cbid == 0xFF) {
				// CbId is not set, set from Seq.
				u8cbid = u8seq & 31;
				pkt.get_psTxDataApp()->u8CbId = u8cbid;
			}

			// set sequence number (automatically counting up)
			if (u8cbid < 32) {
				// user packet 
				pkt.get_psTxDataApp()->u8Seq = u8seq;
			}
			else {
				// system packet (e.g. repeat)
				;
			}

			// set length, from payload size.
			pkt.get_psTxDataApp()->u8Len = pkt.get_payload().size();

			// do put request.
			bool b_tx_req = ToCoNet_bMacTxReq(pkt.get_psTxDataApp());

			// on success, register cbid.
			if (b_tx_req) {
				tx_status._tx_request(u8cbid);

				if (pkt.get_tx_process_immediate()) {
					ToCoNet_Tx_vProcessQueue();
				}
			}
			else {
				u8cbid = 0xff;
			}

			return MWX_APIRET(b_tx_req, u8cbid);
		};

	public:
		// manage tx completion status.
		twenet_tx_cb_management tx_status;
	};


}}
