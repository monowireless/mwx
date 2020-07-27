/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include <jendefs.h>
#include <AppHardwareApi.h>

#include "mwx_periph.hpp"

// generate random numbers
uint32_t random(uint32_t minval, uint32_t maxval) {
	if (minval > maxval) {
		uint32_t t;
		t = minval;
		minval = maxval;
		maxval = t;
	}

	if (maxval <= 0xFFFF) {
		uint16_t r16 = ToCoNet_u32GetRand() & 0xFFFF;
		uint32_t r = ((maxval - minval) * r16) >> 16;
		r = r + minval;
		
		return r;
	}
	else {
		uint32_t r32 = ToCoNet_u32GetRand();
		uint64_t r = ((uint64_t)(maxval - minval) * r32) >> 32;
		r = r + minval;
		
		return (uint32_t)r;
	}
}