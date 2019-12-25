/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "mwx_sensors.hpp"
#include "legacy/sensor_driver.h"
#include "mwx_debug.h"

namespace mwx { inline namespace L1 {
	bool sns_probe_true();
	template <typename DT, void (INIT)(DT*, tsSnsObj*), void (FINAL)(DT*, tsSnsObj*), bool (PROBE)() = sns_probe_true>
    class sns_legacy : public sensor_crtp<sns_legacy<DT,INIT,FINAL,PROBE>> {
		tsSnsObj _snsobj;
		DT _data;

		void _ev_process(tsSnsObj* pObj, teEvent eEv) {
			uint8 u8status_init;

			u8status_init = pObj->u8State;
			pObj->pvProcessSnsObj(pObj, eEv);

			while (pObj->u8State != u8status_init) {
				u8status_init = pObj->u8State;
				pObj->pvProcessSnsObj(pObj, E_EVENT_NEW_STATE);
			}
		}

    public:
		sns_legacy() {
		}

    public:
        bool probe() {
			return PROBE();
        }

        void setup(uint32_t arg1 = 0, uint32_t arg2 = 0) {
			INIT(&_data, &_snsobj);
			//MWX_DebugMsg(0, "{setup:%x}", _snsobj.pvData);
			// _dump();
        }

        void begin(uint32_t arg1 = 0, uint32_t arg2 = 0) {
			if(bSnsObj_isComplete(&_snsobj)) {
				_ev_process(&_snsobj, E_ORDER_KICK);	
			}
			_ev_process(&_snsobj, E_ORDER_KICK);
			//MWX_DebugMsg(0, "{begin:%x}", _snsobj.pvData);
        }

        void process_ev(uint32_t arg1, uint32_t arg2 = 0) {
			_ev_process(&_snsobj, (teEvent)arg1);
			//MWX_DebugMsg(0, "{process_ev:%x}", _snsobj.pvData);
        }

        bool available() {
			bool b = bSnsObj_isComplete(&_snsobj);
			//if (b) _ev_process(&_snsobj, E_ORDER_KICK);
			//MWX_DebugMsg(0, "{available:%x}", &_snsobj);
			return b;
        }

        void end() {
			// once issued, setup() shall be called again.
			FINAL(&_data, &_snsobj);
			//MWX_DebugMsg(0, "{end:%d}", _snsobj.u8TickDelta);
        };

        void on_sleep() {
        }

        void wakeup() {
			_ev_process(&_snsobj, E_EVENT_START_UP);
        }
    
    public:
		DT* _get_data() { return &_data; }
		tsSnsObj& _get_snsobj() { return _snsobj; }

		void _dump() {
			uint8_t *p;
			MWX_DebugMsg(0, "\r\n_data(%x):", &_data);
			p = (uint8_t*)(void*)&_data;
			for (size_t i = 0; i < sizeof(_data); i++, p++) {
				MWX_DebugMsg(0, "%02X ", *p);
			}
			MWX_DebugMsg(0, "\r\n_snsobj(%x):", &_snsobj);
			p = (uint8_t*)(void*)&_snsobj;
			for (size_t i = 0; i < sizeof(_snsobj); i++, p++) {
				MWX_DebugMsg(0, "%02X ", *p);
			}
		}
    };
}}
