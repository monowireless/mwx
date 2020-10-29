/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#include <cstdint>
#include <ToCoNet.h>
#include "networks/mwx_packet_ev.hpp"
#include "networks/mwx_packet_rx.hpp"

#define MWX_APPDEFS_CRTP(c) public mwx::appdefs_crtp<c>, public mwx::processev_crtp<c>

const uint32_t _MWX_EV_ON_SLEEP = 1;
const uint32_t _MWX_EV_ON_WARMBOOT = 2;
const uint32_t _MWX_EV_ON_WAKEUP = 3;
const uint32_t _MWX_EV_ON_CREATE = 4;
const uint32_t _MWX_EV_ON_BEGIN = 5;
const uint32_t _MWX_EV_ON_MESSAGE = 6;

namespace mwx { inline namespace L1 {
	template <class T> class appdefs_crtp;

	/**
	 * appdefs_crtp<T> の仮想化クラス
	 *  - インタフェースメソッドを呼び出すことを目的とする
	 */
	class appdefs_virt {
		void* p_body; // appdefs_crtp<T>* 型
		uint8(*pf_cbTweNet_u8HwInt)(void* pObj, uint32_t u32DeviceId, uint32_t u32ItemBitmap);
		void (*pf_cbTweNet_vHwEvent)(void* pObj, uint32_t u32DeviceId, uint32_t u32ItemBitmap);
		void (*pf_cbTweNet_vMain)(void* pObj);
		void (*pf_cbTweNet_vNwkEvent)(void* pObj, mwx::packet_ev_nwk &pEvNwk);
		void (*pf_cbTweNet_vRxEvent)(void* pObj, mwx::packet_rx& pRx);
		void (*pf_cbTweNet_vTxEvent)(void* pObj, mwx::packet_ev_tx& pEvTx);

		void (*pf_on_event)(void* pObj, uint32_t ev, uint32_t& opt);

	public:
		appdefs_virt() {
			setup();
		}

		~appdefs_virt() {
			if (p_body) delete p_body; // note: only delete memory block, not calling destructor of created object
									   // but to call unuse<T> explicitly.
		}

		// destruct the object (not used function)
		template <class T>
		void unuse() {
			if (p_body) {
				T* p = static_cast<T*>(p_body);
				p_body = nullptr;
				delete p;
			}
		}

		template <class T>
		appdefs_virt& operator = (const T& ref) {
			static_assert(std::is_base_of<appdefs_crtp<T>, T>::value == true, "is not base of appdefs_crtp<T>.");
			p_body = (void*)&ref; // note: never put pointer of appdefs_crtp<T> type, but shall be type T!
								  //       in some case, appdefs_crtp<T>* is not same with T*!!!

			pf_cbTweNet_u8HwInt = ref._cbTweNet_u8HwInt;
			pf_cbTweNet_vHwEvent = ref._cbTweNet_vHwEvent;
			pf_cbTweNet_vMain = ref._cbTweNet_vMain;
			pf_cbTweNet_vNwkEvent = ref._cbTweNet_vNwkEvent;
			pf_cbTweNet_vRxEvent = ref._cbTweNet_vRxEvent;
			pf_cbTweNet_vTxEvent = ref._cbTweNet_vTxEvent;
			pf_on_event = ref._on_event;

			return *this;
		}

		void setup() {
			p_body = nullptr;
			pf_cbTweNet_u8HwInt = nullptr;
			pf_cbTweNet_vHwEvent = nullptr;
			pf_cbTweNet_vMain = nullptr;
			pf_cbTweNet_vNwkEvent = nullptr;
			pf_cbTweNet_vRxEvent = nullptr;
			pf_cbTweNet_vTxEvent = nullptr;

			pf_on_event = nullptr;
		}

		inline void cbToCoNet_vMain(void) {
			if (pf_cbTweNet_vMain != nullptr)
				pf_cbTweNet_vMain(p_body);
		}

		inline void cbToCoNet_vNwkEvent(mwx::packet_ev_nwk& pEvNwk) {
			if (pf_cbTweNet_vNwkEvent != nullptr)
				pf_cbTweNet_vNwkEvent(p_body, pEvNwk);
		}

		inline void cbToCoNet_vRxEvent(mwx::packet_rx &rx) {
			if (pf_cbTweNet_vRxEvent != nullptr) {
				// wrap with twenet
				pf_cbTweNet_vRxEvent(p_body, rx);
			}
		}

		inline void cbToCoNet_vTxEvent(mwx::packet_ev_tx& pEvTx) {
			if (pf_cbTweNet_vTxEvent != nullptr)
				pf_cbTweNet_vTxEvent(p_body, pEvTx);
		}

		inline void cbToCoNet_vHwEvent(uint32_t u32DeviceId, uint32_t u32ItemBitmap) {
			if (pf_cbTweNet_vHwEvent != nullptr)
				pf_cbTweNet_vHwEvent(p_body, u32DeviceId, u32ItemBitmap);
		}

		inline uint8_t cbToCoNet_u8HwInt(uint32_t u32DeviceId, uint32_t u32ItemBitmap) {
			bool bRet = false;
			if (pf_cbTweNet_u8HwInt != nullptr)
				bRet = (bool)pf_cbTweNet_u8HwInt(p_body, u32DeviceId, u32ItemBitmap);
			return bRet;
		}
		
		inline void on_event(uint32_t ev, uint32_t &opt) {
			if (pf_on_event != nullptr)
				pf_on_event(p_body, ev, opt);
		}

		// not type safe, be care!
		template <class T> inline T& cast();
		
		inline operator bool() { return p_body != nullptr;  }
	};


	/**
	 * CRTP手法を用いた基底クラスの実装。
	 * アプリケーションの実装部をシンプルにするため、本クラスに集約。
	 */
	template <class T>
	class appdefs_crtp {
		friend class appdefs_virt;
		
	protected:

	public:
		appdefs_crtp() {}

#if 0
		/**
		 * 簡易版仮想関数を設定(VTweNetCbsへ必要なポインタを格納する)
		 */
		void get_virtual(appdefs_virt& cbs) {
			cbs.pf_cbTweNet_u8HwInt = _cbTweNet_u8HwInt;
			cbs.pf_cbTweNet_vHwEvent = _cbTweNet_vHwEvent;
			cbs.pf_cbTweNet_vMain = _cbTweNet_vMain;
			cbs.pf_cbTweNet_vNwkEvent = _cbTweNet_vNwkEvent;
			cbs.pf_cbTweNet_vRxEvent = _cbTweNet_vRxEvent;
			cbs.pf_cbTweNet_vTxEvent = _cbTweNet_vTxEvent;
			cbs.pf_on_event = _on_event;

			cbs.p_body = (void*)this;
		}
#endif

	private:
		inline static uint8_t _cbTweNet_u8HwInt(void* pObj, uint32_t u32DeviceId, uint32_t u32ItemBitmap) {
			if (pObj != nullptr) {
				auto derived = static_cast<T*>(pObj);
				return derived->cbTweNet_u8HwInt(u32DeviceId, u32ItemBitmap);
			}
			return 0; // not handled
		}

		inline static void _cbTweNet_vHwEvent(void* pObj, uint32_t u32DeviceId, uint32_t u32ItemBitmap) {
			if (pObj != nullptr) {
				auto derived = static_cast<T*>(pObj);
				derived->cbTweNet_vHwEvent(u32DeviceId, u32ItemBitmap);
			}
		}

		inline static void _cbTweNet_vMain(void* pObj) {
			if (pObj != nullptr) {
				auto derived = static_cast<T*>(pObj);
				derived->loop();
			}
		}

		inline static void _cbTweNet_vNwkEvent(void* pObj, mwx::packet_ev_nwk& pEvNwk) {
			if (pObj != nullptr) {
				auto derived = static_cast<T*>(pObj);
				derived->network_event(pEvNwk);
			}
		}

		inline static void _cbTweNet_vRxEvent(void* pObj, mwx::packet_rx& rx) {
			if (pObj != nullptr) {
				auto derived = static_cast<T*>(pObj);
				derived->receive(rx);
			}
		}

		inline static void _cbTweNet_vTxEvent(void* pObj, mwx::packet_ev_tx& pEvTx) {
			if (pObj != nullptr) {
				auto derived = static_cast<T*>(pObj);
				derived->transmit_complete(pEvTx);
			}
		}

		inline static void _on_event(void* pObj, uint32_t ev, uint32_t &opt) {
			if (pObj != nullptr) {
				auto derived = static_cast<T*>(pObj);

				switch(ev) {
					case _MWX_EV_ON_SLEEP: derived->on_sleep(opt); break;
					case _MWX_EV_ON_WARMBOOT: derived->warmboot(opt); break;
					case _MWX_EV_ON_WAKEUP: derived->wakeup(opt); break;
					case _MWX_EV_ON_CREATE: derived->on_create(opt); break;
					case _MWX_EV_ON_BEGIN: derived->on_begin(opt); break;
					case _MWX_EV_ON_MESSAGE: derived->on_message(opt); break;
				}
			}
		}
	};

	template <class T>
	inline T& appdefs_virt::cast() {
		static_assert(std::is_base_of<appdefs_crtp<T>, T>::value == true, "is not base of appdefs_crtp<T>.");

		// casting from void*, carefully.
		return *reinterpret_cast<T*>(p_body);
	}
}}

