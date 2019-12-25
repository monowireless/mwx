/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "mwx_common.hpp"
#include "mwx_periph.hpp"
#include "mwx_debug.h"
namespace mwx { inline namespace L1 {
	class periph_buttons {
		struct tsBtmCtl {
			uint32_t bmPortMask;
			uint32_t bmPort;
			uint32_t bmChangesLast;
			uint32_t bmChanged; // if changed, set bitmap
			uint8_t u8idx;
			uint8_t u8state;
			uint8_t u8count;
			uint8_t u8BmHistoryCount;

			uint32_t* bmHistory;
			uint8_t u8MaxBmHistory;

			uint16_t u16TickDelta;
			uint32_t u32LastTick;

			tsBtmCtl(uint8_t max_history) {
				memset(this, 0, sizeof(tsBtmCtl));

				u8MaxBmHistory = max_history;
				bmHistory = new uint32_t[max_history];
			}
		};

		// internal buffer allocated at setup()
		tsBtmCtl* psBtmCtl;
		bool_t bStarted;

		void _check();

	public:
		static const uint8_t BTM_STAT_INIT_STAGE1 = 0;
		static const uint8_t BTM_STAT_INIT_STAGE2 = 1;
		static const uint8_t BTM_STAT_REGULAR = 0x10;
		static const uint32_t BTM_FIRSTCAPTURE_MASK = 0x80000000;

	public:
		periph_buttons() : psBtmCtl(nullptr) {}

		// if true, it's listening
		operator bool() { return (bool)bStarted; }

		// setup internal buffer, etc. (call once)
		void setup(uint8_t max_history);

		void begin(uint32_t bmPortMask,
				   uint8_t u8HistoryCount,
				   uint16_t tick_delta);
		
		void end();

		void _on_sleep() {}
		void _on_wakeup() {
			// restart (start again)
			if (psBtmCtl != nullptr && bStarted) {
				begin(psBtmCtl->bmPortMask, psBtmCtl->u8BmHistoryCount, psBtmCtl->u16TickDelta);
			}
		}

		inline bool available() {
			return bStarted && (psBtmCtl->bmChanged != 0);
		}

		inline bool tick() {
			if (psBtmCtl != nullptr) {
				uint16_t t_now = millis();
				if (t_now - psBtmCtl->u32LastTick >= psBtmCtl->u16TickDelta) {
					_check();

					psBtmCtl->u32LastTick = t_now;
				}

				if (psBtmCtl->u8state != BTM_STAT_REGULAR) {
					return false;
				}
				else {
					return true;
				}
			}

			return false;
		}

		bool read(uint32_t& u32port, uint32_t& u32changed);
	};
}}
