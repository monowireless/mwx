/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "twecommon.h"
#include "twesysutils.h"
#include "mwx_debug.h"
#include "tweprintf.h"

#ifdef MWX_DEBUGOUT

extern TWE_tsFILE* _psSerial;

static uint8_t _MWX_DebugLevelMin = 0;
static uint8_t _MWX_DebugLevelMax = 0;

void MWX_DebugMsg_SetLevel(uint8_t lv_min, uint8_t lv_max) {
	_MWX_DebugLevelMin = (uint8_t)lv_min;
	_MWX_DebugLevelMax = (uint8_t)lv_max;
}

void MWX_DebugMsg(int lv, const char* fmt, ...) {
	if (_psSerial != NULL) {
		if (lv >= _MWX_DebugLevelMin && lv <= _MWX_DebugLevelMax) {
			va_list va;
			va_start(va, fmt);
			TWE_vfprintf(_psSerial, fmt, va);
			va_end(va);

			_psSerial->fp_flush(_psSerial);
		}
	}
}

void MWX_Panic(uint32_t u32val, const char* fmt, ...) {
	volatile uint32_t _ct = 0;
	va_list va;
	va_start(va, fmt);
	for (;;) {
		if (_psSerial == NULL) {
			TWE_fprintf(_psSerial, "\r\nPANIC(%X): ", u32val);

			va_list va2;
			va_copy(va2, va);
			TWE_vfprintf(_psSerial, fmt, va);
			va_end(va2);
		}

		_ct = 100000;
		while(_ct > 0) { _ct--; }
	}
	va_end(va);
}

#endif // MWX_DEBUGOUT
