/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdint>
#include <cstring>

#include <jendefs.h>
#include <AppHardwareApi.h>

#include "mwx_utils_enum.hpp"

namespace mwx { inline namespace L1 {
	namespace PIN_DIGITAL {
		const uint8_t DIO0 = 0;
		const uint8_t DIO1 = 1;
		const uint8_t DIO2 = 2;
		const uint8_t DIO3 = 3;
		const uint8_t DIO4 = 4;
		const uint8_t DIO5 = 5;
		const uint8_t DIO6 = 6;
		const uint8_t DIO7 = 7;
		const uint8_t DIO8 = 8;
		const uint8_t DIO9 = 9;
		const uint8_t DIO10 = 10;
		const uint8_t DIO11 = 11;
		const uint8_t DIO12 = 12;
		const uint8_t DIO13 = 13;
		const uint8_t DIO14 = 14;
		const uint8_t DIO15 = 15;
		const uint8_t DIO16 = 16;
		const uint8_t DIO17 = 17;
		const uint8_t DIO18 = 18;
		const uint8_t DIO19 = 19;

		const uint8_t DO0 = 0x80;
		const uint8_t DO1 = 0x81;

		const uint8_t COUNT_DIO = 20;
		const uint8_t COUNT_DO = 2;
	}
}}

inline namespace PIN_MODE {
	enum eMWX_DIO_MODE : uint8_t {
		INPUT = 0,
		OUTPUT = 1,
		INPUT_PULLUP = 2,
		INPUT_PULLDOWN = 3, // not supported
		OUTPUT_INIT_HIGH = 4, // set as output and set HIGH.
		OUTPUT_INIT_LOW = 5, // set as output and set LOW.

		WAKE_FALLING = 0x11, // call before sleeping, to set FALLING edge to wake w/o PULLUP.
		WAKE_RISING = 0x12,
		WAKE_FALLING_PULLUP = 0x13,
		WAKE_RISING_PULLUP = 0x14,

		DISABLE_OUTPUT = 0x20,
	};
	
	typedef mwx::enum_wapper<eMWX_DIO_MODE> E_PIN_MODE;
}

inline namespace PIN_STATE {
	enum eMWX_DIO_HIGHLOW : uint8_t {
		LOW = 0,
		HIGH = 1
	};

	typedef mwx::enum_wapper<eMWX_DIO_HIGHLOW> E_PIN_STATE;
}

inline namespace PIN_INT_MODE {
	enum eMWX_DIO_INTMODE : uint8_t {
		//LOW = 0,
		//CHANGE = 1, // no change here
		FALLING = 2,
		RISING = 3,
	};

	typedef mwx::enum_wapper<eMWX_DIO_INTMODE> E_PIN_INT_MODE;
}

/**
 * ENUM DIO STATE/INT STATE
 * avoid conflict of enum LOW
 */
//typedef mwx::enum_combiner<teMWX_DIO_HIGHLOW,teMWX_DIO_INTMODE> teDIO_STATE;

/**
 * set DIO pin modes
 * avoid conflict of enum LOW
 */
void pinMode(uint8_t u8pin, E_PIN_MODE mode);

/**
 * set DIO output state
 */
static inline void digitalWrite(uint8_t u8pin, E_PIN_STATE ulVal) {
	if (u8pin < mwx::PIN_DIGITAL::COUNT_DIO) {
		uint32_t b = 1UL << u8pin;

		if (ulVal == PIN_STATE::LOW) {
			vAHI_DioSetOutput(0, b);
		}
		else {
			vAHI_DioSetOutput(b, 0);
		}
	}
	else if (u8pin == 0x80 || u8pin == 0x81) {
		uint32_t b = 1UL << (u8pin & 0x7F);

		if (ulVal == PIN_STATE::LOW) {
			vAHI_DoSetDataOut(0, b);
		}
		else {
			vAHI_DoSetDataOut(b, 0);
		}
	}
}

/**
 * read DIO state
 */
static inline E_PIN_STATE digitalRead(uint8_t u8pin) {
	uint32_t b = 1UL << u8pin;
	return (u32AHI_DioReadInput() & b) ? PIN_STATE::HIGH : PIN_STATE::LOW;
}

/**
 * read DIO bitmap (read DIO0..DIO19)
 */
static inline uint32_t digitalReadBitmap() {
	return  (u32AHI_DioReadInput()) & ((1 << mwx::PIN_DIGITAL::COUNT_DIO) - 1);
}

// GPIO interrupt
void _MWX_periph_init();
void attachIntDio(uint8_t u8pin, E_PIN_INT_MODE mode);
void detachIntDio(uint8_t u8pin);

