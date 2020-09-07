/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "../mwx_utils_smplque.hpp"
#include "mwx_settings.hpp"
#include "mwx_stgs_standard_core.h"

namespace mwx { inline namespace L1 {

	class StgsStandard : MWX_APPDEFS_CRTP(StgsStandard)
	{
		friend class _serial;

	public: // constants
		settings set;

		// serial handling sub queue.
		class _serial {
			mwx::smplque<uint8_t, mwx::alloc_local<uint8_t,128>> _uart_que;
		public:
			_serial() : _uart_que() {}
			void push(uint8_t c) { _uart_que.push(c); }
			bool available() { return !_uart_que.empty(); }
			bool is_full() { return _uart_que.is_full(); }
			int read() { return (int)_uart_que.pop_front(); }

			static bool _available(void *obj) {
				if (obj) {
					return reinterpret_cast<_serial*>(obj)->available();
				} else {
					return false;
				}
			}

			static int _read(void *obj) {
				int ret = -1;
				if (obj) {
					_serial *pser = reinterpret_cast<_serial*>(obj);
					if (pser->available()) {
						ret = pser->read();
					}
				}
				return ret;
			}
		} serial;

	private: // local vars
		TWEINTRCT_tsContext* _psIntr;
		serial_jen::SURR_OBJ _ser_surr;
		
	public: // common header
		static const uint8_t TYPE_ID = mwx::SETTINGS::STANDARD;

		// load common definition for handlers
		#define __MWX_APP_CLASS_NAME StgsStandard
		#include "../_mwx_cbs_hpphead.hpp"
		#undef __MWX_APP_CLASS_NAME

	public: // constructor, etc...
		StgsStandard() : _psIntr(0), _ser_surr{}, set(), serial() {}

		// begin method (if necessary, configure object here)
		void _setup();

		// begin method (if necessary, start object here)
		void _begin();

	public:
		// TWENET callback handler (mandate)
		void loop() {
			TWEINTRCT_vHandleSerialInput(); // check uart char from twesettings lib.
		}

		void on_sleep(uint32_t & val) { }

		void warmboot(uint32_t & val) { }
		void wakeup(uint32_t & val) { }

		void on_create(uint32_t& val) { _setup();  }
		void on_begin(uint32_t& val) { _begin(); }
		void on_message(uint32_t& val) { }

	public: // never called the following as hardware class, but define it!
		void network_event(mwx::packet_ev_nwk& ev) {}
		void receive(mwx::packet_rx& rx) {}
		void transmit_complete(mwx::packet_ev_tx& ev) {}

	// class specific defs
	public: // exported methods
		// load data form EEPROM w/ slot number.
		void reload(uint8_t slot = 0xFF); // load slot information.

		// initial settings (during setup())
private:
		void set_appname(const char*name);
		void set_default_appid(uint32_t u32appid);
		void set_ch_multi();
		bool _get_ch_multi();

public:
		// << operator settings
		StgsStandard& operator << (SETTINGS::appname&& v) { set_appname(v._val); return *this; }
		StgsStandard& operator << (SETTINGS::appid_default&& v) { set_default_appid(v._val); return *this; }
		StgsStandard& operator << (SETTINGS::ch_multi&& v) { set_ch_multi(); return *this; }
		StgsStandard& operator << (SETTINGS::open_at_start&& v);

		// replace name/desc of item
		void replace_item_name_desc(const TWESTG_tsMsgReplace* val) {
			_psIntr->msgReplace = val;
		}

		// hide some item(s) (note: number of configuration is limited to (SIZE_SETSTD_CUST_COMMON-9)/2)
		uint8_t* _hide_items(uint8_t*p, uint8_t*e) {
			return p;
		}
		
		template <typename... Tail>
		uint8_t* _hide_items(uint8_t*p, uint8_t*e, uint8_t head, Tail&&... tail) {
			if (p + 2 < e) {
				*p++ = uint8_t(head);
				*p++ = TWESTG_DATATYPE_UNUSE;
				return _hide_items(p, e, std::forward<Tail>(tail)...);
			} else {
				return nullptr;
			}
		}
		
		template <typename... Tail>
		uint8_t* _hide_items(uint8_t*p, uint8_t*e, E_STGSTD_SETID head, Tail&&... tail) {
			return _hide_items(p, e, uint8_t(head), std::forward<Tail>(tail)...);
		}

		template <typename... Tail>
		bool hide_items(Tail&&... tail) {
			uint8* p = SETSTD_CUST_COMMON + SETSTD_CUST_COMMON[0] + 1;
			uint8* e = std::end(SETSTD_CUST_COMMON);
			e = _hide_items(p, e, std::forward<Tail>(tail)...);
			if (e != nullptr) {
				SETSTD_CUST_COMMON[0] = (e - SETSTD_CUST_COMMON) - 1;
				return true;
			} else 
				return false;
		}

		// data acquisition (this is not efficient way to frequent query, since find_by_id() searches array linerly)
		uint32_t u32appid() { return set.find_by_id(E_TWESTG_DEFSETS_APPID)->get_value()->uDatum.u32; }
		uint8_t u8devid() { return set.find_by_id(E_TWESTG_DEFSETS_LOGICALID)->get_value()->uDatum.u8; }
		uint8_t u8ch() { return _get_ch_multi() ? 0 : set.find_by_id(E_TWESTG_DEFSETS_CHANNEL)->get_value()->uDatum.u8; }
		uint32_t u32chmask() { return _get_ch_multi() ? (set.find_by_id(E_TWESTG_DEFSETS_CHANNELS_3)->get_value()->uDatum.u16) << 11 : 0;}
		uint8_t u8power() { return set.find_by_id(E_TWESTG_DEFSETS_POWER_N_RETRY)->get_value()->uDatum.u8 & 0x0F; }
		uint8_t u8retry() { return set.find_by_id(E_TWESTG_DEFSETS_POWER_N_RETRY)->get_value()->uDatum.u8 >> 4; }
		uint32_t u32opt1() { return set.find_by_id(E_TWESTG_DEFSETS_OPTBITS)->get_value()->uDatum.u32; }
		uint32_t u32opt2() { return set.find_by_id(E_TWESTG_DEFSETS_OPT_DWORD1)->get_value()->uDatum.u32; }
		uint32_t u32opt3() { return set.find_by_id(E_TWESTG_DEFSETS_OPT_DWORD2)->get_value()->uDatum.u32; }
		uint32_t u32opt4() { return set.find_by_id(E_TWESTG_DEFSETS_OPT_DWORD3)->get_value()->uDatum.u32; }
		uint8_t u8encmode() { return set.find_by_id(int(E_STGSTD_SETID::ENC_MODE))->get_value()->uDatum.u8; }
		const uint8_t * pu8enckeystr() { return set.find_by_id(int(E_STGSTD_SETID::ENC_KEY_STRING))->get_value()->uDatum.pu8; }

	private:
		void _reload(bool);
		static void _vProcessInputByte(TWEINTRCT_tsContext *, int16);
	};
}}
