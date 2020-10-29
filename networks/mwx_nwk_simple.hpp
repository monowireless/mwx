/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "mwx_networks.hpp"
#include "../mwx_twenet.hpp"

#include <utility>

namespace mwx { inline namespace L1 {
	struct NwkSimple_Config {
		uint8_t u8Lid;
		uint8_t u8RepeatMax;
		uint8_t u8Type;
		uint8_t u8Cmd; // b0..b3:Packet Cmd, b7:encrypt b6:enc+rcv plain
		bool_t bRcvNwkLess;

		uint8_t get_pkt_cmd_part() { return u8Cmd & 0x07; }
		bool is_mode_pkt_encrypt() { return (u8Cmd & 0x80); }
		bool is_mode_rcv_plain() { return is_mode_pkt_encrypt() && (u8Cmd & 0x40); }
	};
	
	template <class T>
	class packet_tx_nwk_simple : public mwx::packet_tx {
		T *_this;

	public:
		packet_tx_nwk_simple(T *p) : _this(p) {}

		operator mwx::packet_tx& () { return *static_cast<mwx::packet_tx*>(this); }

		MWX_APIRET transmit() {
			return _this->transmit(*this);
		}
	public:
		// using mwx::packet_tx::operator <<;
		// override some operator by changing return class.
		inline packet_tx_nwk_simple& operator << (mwx::tx_addr&& a) {
			mwx::packet_tx::operator<<(std::forward<mwx::tx_addr&&>(a));
			return *this;
		}

		inline packet_tx_nwk_simple& operator << (mwx::tx_retry&& a) {
			mwx::packet_tx::operator<<(std::forward<mwx::tx_retry&&>(a));
			return *this;
		}

		inline packet_tx_nwk_simple& operator << (mwx::tx_packet_delay&& a) {
			mwx::packet_tx::operator<<(std::forward<mwx::tx_packet_delay&&>(a));
			return *this;
		}

		inline packet_tx_nwk_simple& operator << (mwx::tx_process_immediate&& a) {
			mwx::packet_tx::operator<<(std::forward<mwx::tx_process_immediate&&>(a));
			return *this;
		}
		
		// remove some operators
		inline mwx::packet_tx& operator << (mwx::tx_addr_broadcast&&) = delete;
		inline mwx::packet_tx& operator << (mwx::tx_ack_required&&) = delete;
		inline mwx::packet_tx& operator << (mwx::tx_packet_type_id&&) = delete;
	};

	class NwkSimple : MWX_APPDEFS_CRTP(NwkSimple)
	{
		friend class mwx::packet_rx;
		friend class mwx::packet_tx;

	public:
		static const uint8_t TYPE_ID = mwx::NETWORK::SIMPLE;

		// load common definition for handlers
#       define __MWX_APP_CLASS_NAME NwkSimple
#       include "../_mwx_cbs_hpphead.hpp"
#       undef __MWX_APP_CLASS_NAME

	private:
		static const uint8_t CBID_MASK = 31;
		duplciate_checker _dupchk;
		NwkSimple_Config _config;

		uint8_t _u8cbid_rpt;
		uint8_t _u8cbid;

		// duplicate checker (from TWENET C library)
		void _init_dup_check();

	public:
		// constructor
		NwkSimple() : _config{ 0 }, _u8cbid_rpt(0), _u8cbid(0) {}

		// begin method (if necessary, start object here)
		void begin();

	public: // TWENET callback handler (mandate)
		void network_event(mwx::packet_ev_nwk& pEvNwk) {
			// so far, all network events are ignored.
			pEvNwk._network_type = _config.u8Type;
			pEvNwk._network_handled = true;
		}

		void transmit_complete(mwx::packet_ev_tx& pEvTx) {
			// if user sent packet, report to user class. otherwise (like repeat packets) not.
			pEvTx._network_type = _config.u8Type;
			pEvTx._network_handled = (pEvTx.u8CbId > CBID_MASK);
		}

		void receive(mwx::packet_rx& rx);

	public:


		MWX_APIRET transmit(packet_tx_nwk_simple<NwkSimple>& pkt);
		
		/**
		 * @fn	mwx::packet_tx NwkSimple::prepare_tx_packet()
		 *
		 * @brief	returns tx packet for network transmit.
		 *
		 * @returns	A mwx::packet_tx.
		 */
		packet_tx_nwk_simple<NwkSimple> prepare_tx_packet() {
			packet_tx_nwk_simple<NwkSimple> pkt(this);

			if (_config.u8Type == 0x01) {
				pkt.get_payload().reserve_head(_get_header_size());
				pkt.get_psTxDataApp()->u8Retry = 2;
			}

			return pkt;
		}

	private:
		/**
		 * @fn	inline int NwkSimple::_get_header_size()
		 *
		 * @brief	Gets header size of packet payload.
		 *
		 * @returns	The header size.
		 */
		inline int _get_header_size() {
			return 11; // type == 1
		}

	public: // never called the following as hardware class, but define it!
		void loop() {}
		void warmboot(uint32_t& val) {}
		void wakeup(uint32_t& val) {}

	public: // implement system message call
		void on_sleep(uint32_t& val) {}
		void on_create(uint32_t& val);
		void on_begin(uint32_t& val);
		void on_message(uint32_t& val) {}

	public: // configurations (setups)
		struct logical_id {
			uint16_t _val;
			logical_id(uint8_t val) : _val(val) {}
		};
		NwkSimple& operator << (logical_id&& v) { _config.u8Lid = v._val; return *this;  }

		struct repeat_max {
			uint8_t _val;
			repeat_max(uint8_t val) : _val(val) {}
		};
		NwkSimple& operator << (repeat_max&& v) { _config.u8RepeatMax = v._val; return *this; }

		struct network_type {
			uint8_t _val;
			network_type(uint16_t val) : _val(val) {}
		};
		NwkSimple& operator << (network_type&& v) { _config.u8Type = v._val; return *this; }

		struct secure_pkt {
			const uint8_t *_pukey;
			bool _b_recv_plain_pkt;
			secure_pkt(const uint8_t *pukey, bool b_recv_plain_pkt = false) : _pukey(pukey), _b_recv_plain_pkt(b_recv_plain_pkt)  {}
		};
		NwkSimple& operator << (secure_pkt&& v) {
			the_twelite.register_crypt_key((uint8*)v._pukey); // set enc key
			_config.u8Cmd |= 0x80; // enc mode
			if (v._b_recv_plain_pkt) _config.u8Cmd |= 0x40; // rcv plain
			return *this;
		}

		struct receive_nwkless_pkt {
			bool _b;
			receive_nwkless_pkt(bool b = true) : _b(b) {}
		};
		NwkSimple& operator << (receive_nwkless_pkt&& v) { _config.bRcvNwkLess = v._b; }

		struct dup_check {
			uint8_t _maxnodes;
			uint16_t _timeout_ms;
			uint8_t _tickscale;

			dup_check(uint8_t maxnodes = 0, uint16_t timeout_ms = 0, uint8_t tickscale = 0) :
					_maxnodes(maxnodes), _timeout_ms(timeout_ms), _tickscale(tickscale) {}
		};
		NwkSimple& operator << (dup_check&& v) {
			_dupchk.setup(v._maxnodes, v._timeout_ms, v._tickscale);
			return *this;
		}
	};

}}
