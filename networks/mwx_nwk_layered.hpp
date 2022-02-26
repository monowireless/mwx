/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "mwx_networks.hpp"
#include "../mwx_twenet.hpp"
#include "../settings/mwx_stgs_standard.hpp"

#include <utility>

namespace mwx { inline namespace L1 {
	struct NwkLayered_Config {
		uint8_t u8Role; // used for network role.
		uint8_t u8Type;
		uint8_t u8Cmd; // b0..b3:Packet Cmd, b7:encrypt b6:enc+rcv plain
		bool_t bRcvNwkLess;

		uint8_t get_pkt_cmd_part() { return u8Cmd & 0x07; }
		bool is_mode_pkt_encrypt() { return (u8Cmd & 0x80); }
		bool is_mode_rcv_plain() { return is_mode_pkt_encrypt() && (u8Cmd & 0x40); }
	};
	
	template <class T>
	class packet_tx_nwk_layered : public mwx::packet_tx {
		T *_this;

	public:
		packet_tx_nwk_layered(T *p) : _this(p) {}

		//operator mwx::packet_tx& () { return *static_cast<mwx::packet_tx*>(this); }
				// has -Wclass-conversion error, may not be necessary. to be removed.

		MWX_APIRET transmit() {
			return _this->transmit(*this);
		}
	public:
		// using mwx::packet_tx::operator <<;
		// override some operator by changing return class.
		inline packet_tx_nwk_layered& operator << (mwx::tx_addr&& a) {
			mwx::packet_tx::operator<<(std::forward<mwx::tx_addr&&>(a));
			return *this;
		}

		inline packet_tx_nwk_layered& operator << (mwx::tx_retry&& a) {
			mwx::packet_tx::operator<<(std::forward<mwx::tx_retry&&>(a));
			return *this;
		}

		inline packet_tx_nwk_layered& operator << (mwx::tx_packet_delay&& a) {
			mwx::packet_tx::operator<<(std::forward<mwx::tx_packet_delay&&>(a));
			return *this;
		}

		inline packet_tx_nwk_layered& operator << (mwx::tx_process_immediate&& a) {
			mwx::packet_tx::operator<<(std::forward<mwx::tx_process_immediate&&>(a));
			return *this;
		}
		
		// remove some operators
		inline mwx::packet_tx& operator << (mwx::tx_addr_broadcast&&) = delete;
		inline mwx::packet_tx& operator << (mwx::tx_ack_required&&) = delete;
		inline mwx::packet_tx& operator << (mwx::tx_packet_type_id&&) = delete;
	};

	class NwkLayered : MWX_APPDEFS_CRTP(NwkLayered)
	{
		friend class mwx::packet_rx;
		friend class mwx::packet_tx;

	public:
		static const uint8_t TYPE_ID = mwx::NETWORK::LAYERED;

		// load common definition for handlers
#       define __MWX_APP_CLASS_NAME NwkLayered
#       include "../_mwx_cbs_hpphead.hpp"
#       undef __MWX_APP_CLASS_NAME

	private:
		static const uint8_t CBID_MASK = 31;
		NwkLayered_Config _config;

		tsToCoNet_Nwk_Context *_pContextNwk; //!< ネットワークコンテキスト
		tsToCoNet_NwkLyTr_Config _sNwkLayerTreeConfig; //!< LayerTree の設定情報

	public:
		// constructor
		NwkLayered() : _pContextNwk(nullptr) {
			memset(&_config, 0, sizeof(_config));
			_config.u8Role = ROLE_PARENT; // default is parent.
			_config.u8Type = 0x80;

			memset(&_sNwkLayerTreeConfig, 0, sizeof(_sNwkLayerTreeConfig));
		}

		// begin method (if necessary, start object here)
		void begin();

	public: // TWENET callback handler (mandate)
		void network_event(mwx::packet_ev_nwk& pEvNwk) {
			// so far, all network events are passed.
			pEvNwk._network_type = _config.u8Type;
			pEvNwk._network_handled = true;
		}

		void transmit_complete(mwx::packet_ev_tx& pEvTx) {
			// if user sent packet, report to user class. otherwise (like repeat packets) not.
			if (the_mac.is_cbid_nwk(pEvTx.u8CbId)) {
				pEvTx._network_type = _config.u8Type;
				pEvTx._network_handled = true;
			}
		}

		void receive(mwx::packet_rx& rx);

	public:
		MWX_APIRET transmit(packet_tx_nwk_layered<NwkLayered>& pkt);
		
		/**
		 * @fn	mwx::packet_tx NwkLayered::prepare_tx_packet()
		 *
		 * @brief	returns tx packet for network transmit.
		 *
		 * @returns	A mwx::packet_tx.
		 */
		packet_tx_nwk_layered<NwkLayered> prepare_tx_packet() {
			packet_tx_nwk_layered<NwkLayered> pkt(this);

			return pkt;
		}


	public: // never called the following as hardware class, but define it!
		void loop() {}
		void warmboot(uint32_t& val) {}
		void wakeup(uint32_t& val) {
			if (_pContextNwk) {
				ToCoNet_Nwk_bResume(_pContextNwk);
			}
		}

	public: // implement system message call
		void on_sleep(uint32_t& val) {}
		void on_create(uint32_t& val);
		void on_begin(uint32_t& val);
		void on_message(uint32_t& val) {}

	public: // configurations (setups)
		struct network_role {
			uint8_t _val;
			network_role(uint8_t val) : _val(val) {}
		};
		NwkLayered& operator << (network_role&& v) { _config.u8Role = v._val; return *this; }

		struct secure_pkt {
			const uint8_t *_pukey;
			bool _b_recv_plain_pkt;
			secure_pkt(const uint8_t *pukey = nullptr, bool b_recv_plain_pkt = false) : _pukey(pukey), _b_recv_plain_pkt(b_recv_plain_pkt)  {}
		};
		NwkLayered& operator << (secure_pkt&& v) {
			if (v._pukey != nullptr) {
				the_twelite.register_crypt_key((uint8*)v._pukey); // set enc key
			}

			_config.u8Cmd |= 0x80; // enc mode
			if (v._b_recv_plain_pkt) _config.u8Cmd |= 0x40; // rcv plain
			return *this;
		}

		struct receive_nwkless_pkt {
			bool _b;
			receive_nwkless_pkt(bool b = true) : _b(b) {}
		};
		NwkLayered& operator << (receive_nwkless_pkt&& v) { _config.bRcvNwkLess = v._b; return *this; }


public:
		const NwkLayered_Config& get_config() const { return _config; }

public:
		static const uint8_t ROLE_ENDDEVICE = TOCONET_NWK_ROLE_ENDDEVICE;
		static const uint8_t ROLE_ROUTER = TOCONET_NWK_ROLE_ROUTER;
		static const uint8_t ROLE_PARENT = TOCONET_NWK_ROLE_PARENT;

		tsToCoNet_Nwk_Context *_get_nwk_context() { return _pContextNwk; }
	};

}}
