/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <cstring>
#include <cstdint>

#include "jendefs.h"
#include <AppHardwareApi.h>

#include "Interrupt.h"

#include "mwx_appcore.hpp"
#include "mwx_periph_buttons.hpp"
#include "mwx_debug.h"

#undef READ_IN_CRITSEC // if use check is at INT handler, should set.
/****************************************************************************/
/***        Implementation                                                ***/
/****************************************************************************/

void mwx::periph_buttons::setup(uint8_t max_history) {
	// initialize the context
	if (psBtmCtl == nullptr) {
		psBtmCtl = (tsBtmCtl*)new tsBtmCtl(max_history); 
	}
}

void mwx::periph_buttons::begin(
			uint32_t bmPortMask,
			uint8_t u8HistoryCount,
			uint16_t tick_delta) {

	if (psBtmCtl == nullptr) return;

	// initialize the context
	uint32_t* save_bmHistory = psBtmCtl->bmHistory;
	uint8_t save_u8BmHistoryCount = psBtmCtl->u8BmHistoryCount;
	uint8_t save_u8MaxBmHistory = psBtmCtl->u8MaxBmHistory;

	memset(psBtmCtl, 0, sizeof(tsBtmCtl));
	memset(save_bmHistory, 0, sizeof(uint32_t) * save_u8BmHistoryCount);
	psBtmCtl->bmHistory = save_bmHistory;
	psBtmCtl->u8BmHistoryCount = save_u8BmHistoryCount;
	psBtmCtl->u8MaxBmHistory = save_u8MaxBmHistory;
	
	// store port mask
	psBtmCtl->bmPortMask = bmPortMask;

	// history count
	if (u8HistoryCount > psBtmCtl->u8MaxBmHistory)
		u8HistoryCount = psBtmCtl->u8MaxBmHistory;

	psBtmCtl->u8BmHistoryCount = u8HistoryCount;

	// tick count
	psBtmCtl->u16TickDelta = tick_delta;
	psBtmCtl->u32LastTick = millis();

	// Set DIO as INPUT
	vAHI_DioSetDirection(psBtmCtl->bmPortMask, 0);

	// start immediately.
	bStarted = (uint8_t)true;

	// exits
	return;
}

void mwx::periph_buttons::end() {
	if (psBtmCtl == nullptr) return;
	bStarted = (uint8_t)false;
}

bool mwx::periph_buttons::read(uint32_t& u32port, uint32_t& u32changed) {
	if (psBtmCtl == nullptr) return false;

	u32port = 0;
	u32changed = 0;

	// store data in critical section.
#ifdef READ_IN_CRITSEC
	MICRO_INT_STORAGE;
	MICRO_INT_ENABLE_ONLY(0);
#endif
	u32port = (~psBtmCtl->bmPort & psBtmCtl->bmPortMask) | (psBtmCtl->bmPort & 0x80000000);
	u32changed = psBtmCtl->bmChanged;
	psBtmCtl->bmChanged = 0; // clear changed flag.
#ifdef READ_IN_CRITSEC
	MICRO_INT_RESTORE_STATE();
#endif

	return true;
}

/** @ingroup grp_Utils_BTM
 * 割り込みハンドラ。IO入力と連照判定を行う。
 */
void mwx::periph_buttons::_check() {
	if (psBtmCtl == nullptr) return;

	int i;

	// update index
	psBtmCtl->u8idx++;
	if (psBtmCtl->u8idx >= psBtmCtl->u8BmHistoryCount) {
		psBtmCtl->u8idx = 0;
	}

	// read all IO ports and store it into the history table
	uint32_t bmRead = u32AHI_DioReadInput(); // Api returns as Hi=1, Lo=0
	
	bmRead = ~bmRead & psBtmCtl->bmPortMask; // Now Hi=0, Lo=1
	psBtmCtl->bmHistory[psBtmCtl->u8idx] = bmRead;

	// check changed bits
	uint32_t bmChanged = 0; // port bitmap, if bit is not set, history value have all the same (no change).
	for (i = 0; i < psBtmCtl->u8BmHistoryCount - 1; i++) {
		bmChanged |= psBtmCtl->bmHistory[i] ^ psBtmCtl->bmHistory[i + 1];
	}
	
	// wait for start status
	switch (psBtmCtl->u8state) {
	case BTM_STAT_INIT_STAGE1: // initial stage 1 (fill history)
		psBtmCtl->u8count++;
		if (psBtmCtl->u8count > psBtmCtl->u8BmHistoryCount) {
			psBtmCtl->u8state = BTM_STAT_INIT_STAGE2;
		}
		break;
	case BTM_STAT_INIT_STAGE2: // initial stage 2 (wait for stabilize)
		if (bmChanged)
			break;
		else
			psBtmCtl->u8state = BTM_STAT_REGULAR;

		psBtmCtl->bmChanged = (psBtmCtl->bmPortMask | BTM_FIRSTCAPTURE_MASK);
		psBtmCtl->bmPort = psBtmCtl->bmHistory[psBtmCtl->u8idx];
		psBtmCtl->bmChangesLast = 0;
		break;
	case BTM_STAT_REGULAR: // regular update
		{
			// the mask indicates, previously fluctuated, but now stable.
			uint32_t bmNewlyUpdated = psBtmCtl->bmChangesLast & ~bmChanged;

			// find new changed port value.
			uint32_t bmPortUpdated = psBtmCtl->bmHistory[psBtmCtl->u8idx]
					& bmNewlyUpdated;
			psBtmCtl->bmPort &= ~bmNewlyUpdated;
			psBtmCtl->bmPort |= bmPortUpdated;

			// update bmChangesLast
			psBtmCtl->bmChangesLast = bmChanged;

			// record changed ports
			psBtmCtl->bmChanged |= bmNewlyUpdated;
		}
		break;
	default:
		psBtmCtl->u8state = BTM_STAT_INIT_STAGE1; // back to initial
		break;
	}
}
