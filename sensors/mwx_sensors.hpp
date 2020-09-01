/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../mwx_debug.h"

namespace mwx { inline namespace L1 {
	namespace SENSOR {
		// sensor name table

		// internal use
		typedef bool (*PFN_METHOD)(void* pobj, uint8_t method, uint32_t arg1, uint32_t arg2);

		const uint8_t _SENSOR_METHOD_SETUP = 1;
		const uint8_t _SENSOR_METHOD_BEGIN = 2;
		const uint8_t _SENSOR_METHOD_EVENT = 3;
		const uint8_t _SENSOR_METHOD_AVAIL = 4;
		const uint8_t _SENSOR_METHOD_END = 5;
		const uint8_t _SENSOR_METHOD_PROBE = 7;

		const uint8_t _SENSOR_METHOD_WAKEUP = 10;
		const uint8_t _SENSOR_METHOD_ON_SLEEP = 11;
	}

	template <class T> class sensor_crtp;

	
	class sensor_virt {
#if 0 // note: sensor_virt is not used so far
		void* _p_obj; // sensor_crtp<T>*
		SENSOR::PFN_METHOD _pf_act;
		uint16_t _type_id;
	public:
		// constructor
		sensor_virt() : _p_obj(nullptr), _pf_act(nullptr), _type_id(0) {}

		template <class T>
		sensor_virt(T& ref) {
			operator =(ref);
		}

		template <class T>
		sensor_virt& operator =(T& ref) {
			static_assert(std::is_base_of<sensor_crtp<T>, T>::value == true, "is not base of sensor_crtp<T>.");

			_p_obj = (void*)ref;
			_pf_act = ref._act;
			_type_id = T::TYPE_ID;

			return *this;
		}

		// set obj
		void _set_obj(void* pobj, SENSOR::PFN_METHOD pmethod, uint16_t tyid) {
			_p_obj = pobj;
			_pf_act = pmethod;
			_type_id = tyid;
		}

		// type cast
		template <class T> T& cast();

		// interface
		void setup(uint32_t arg1 = 0, uint32_t arg2 = 0) {
			_pf_act(_p_obj, SENSOR::_SENSOR_METHOD_SETUP, arg1, arg2);
		}

		void begin(uint32_t arg1 = 0, uint32_t arg2 = 0) {
			_pf_act(_p_obj, SENSOR::_SENSOR_METHOD_BEGIN, arg1, arg2);
		}

		void process_ev(uint32_t arg1 = 0, uint32_t arg2 = 0) {
			_pf_act(_p_obj, SENSOR::_SENSOR_METHOD_EVENT, arg1, arg2);
		}

		bool available() {
			return _pf_act(_p_obj, SENSOR::_SENSOR_METHOD_AVAIL, 0, 0);
		}

		void end() {
			_pf_act(_p_obj, SENSOR::_SENSOR_METHOD_BEGIN, 0, 0);
		}

		bool probe() {
			return _pf_act(_p_obj, SENSOR::_SENSOR_METHOD_PROBE, 0, 0);
		}

		void wakeup() {
			_pf_act(_p_obj, SENSOR::_SENSOR_METHOD_WAKEUP, 0, 0);
		}

		void on_sleep() {
			_pf_act(_p_obj, SENSOR::_SENSOR_METHOD_ON_SLEEP, 0, 0);
		}
#endif
	};

	template <class T>
	class sensor_crtp {
		friend class sensor_virt;
	public:
		static MWX_APIRET _act(void* pobj, uint8_t method, uint32_t arg1, uint32_t arg2) {
			MWX_APIRET ret = true;
			if (pobj != nullptr) {

				T* derived = static_cast<T*>(pobj);

				switch (method) {
				case SENSOR::_SENSOR_METHOD_SETUP:
					ret = derived->setup(arg1, arg2);
					break;
				case SENSOR::_SENSOR_METHOD_BEGIN:
					derived->begin(arg1, arg2);
					break;
				case SENSOR::_SENSOR_METHOD_EVENT:
					ret = derived->process_ev(arg1, arg2);
					break;
				case SENSOR::_SENSOR_METHOD_AVAIL:
					ret = derived->available();
					break;
				case SENSOR::_SENSOR_METHOD_END:
					derived->end();
					break;
				case SENSOR::_SENSOR_METHOD_PROBE:
					ret = derived->probe();
					break;
				case SENSOR::_SENSOR_METHOD_WAKEUP:
					derived->wakeup();
					break;
				case SENSOR::_SENSOR_METHOD_ON_SLEEP:
					derived->on_sleep();
					break;
				}
			}
			return ret;
		}
	};

#if 0
	// type cast
	template <class T>
	T& sensor_virt::cast() {
		static_assert(std::is_base_of<sensor_crtp<T>, T>::value == true, "is not base of sensor_crtp<T>.");

		if (_p_obj != nullptr && T::TYPE_ID == _type_id) {
			T* p = reinterpret_cast<T*>(_p_obj);
			return *p;
		}
		else {
			T* p = (T*)((void*)0xdeadbeef);
			MWX_Panic(0, "{Panic!sensor_virt::cast mismatch id %d->%d}", _type_id, T::TYPE_ID);
			return *p;
		}
	}
#endif
}}
