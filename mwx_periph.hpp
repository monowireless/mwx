/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#include <ToCoNet.h>

#include "twesysutils.h"
#include "tweserial.h"

extern "C" TWE_tsFILE* _psSerial;

/**
 * tickcount [ms]
 */
static inline uint32_t millis() { return u32TickCount_ms; }

/**
 * polling wait for given ms.
 */
static inline void delay(uint32_t ms) {
	if (sToCoNet_Context.bToCoNetStarted) {
		TWESYSUTL_vWaitPoll(ms);
	}
	else {
		// inaccurate method
		volatile uint32_t ct = ms * 1625 * 2; // for 32Mhz
		while (ct > 0) --ct;
	}
}

/**
 * polling wait for given micro seconds.
 */
static inline void delayMicroseconds(uint32_t us) {
	if (sToCoNet_Context.bToCoNetStarted) { // DEBUGGING
		TWESYSUTL_vWaitPollMicro(us);
	}
	else {
		// inaccurate method
		volatile uint32_t ct = (us + us / 2 + us / 8) * 2; // for 32Mhz
		while (ct > 0) --ct;
	}
}

// random numbers (should move other hpp file)
uint32_t random(uint32_t minval, uint32_t maxval = 0);

/** @brief	peripherals interrupt record mask */
extern uint32_t _periph_availmap;
const uint32_t _PERIPH_AVAIL_TIMER0 = (1UL << 0);
const uint32_t _PERIPH_AVAIL_TIMER1 = (1UL << 1);
const uint32_t _PERIPH_AVAIL_TIMER2 = (1UL << 2);
const uint32_t _PERIPH_AVAIL_TIMER3 = (1UL << 3);
const uint32_t _PERIPH_AVAIL_TIMER4 = (1UL << 4);
const uint32_t _PERIPH_AVAIL_TICKTIMER = (1UL << 5);

extern uint32_t _twenet_mod_registered;
const uint32_t _TWENET_MOD_DUPCHK = (1UL << 0);
const uint32_t _TWENET_MOD_BEGIN_PROC = (1UL << 31); // this is not MOD, if begin() called, then set to 1.
