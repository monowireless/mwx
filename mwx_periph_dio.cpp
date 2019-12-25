/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include <cstdint>

#include <jendefs.h>
#include <AppHardwareApi.h>

#include <ToCoNet.h>

#include "mwx_debug.h"
#include "mwx_periph_dio.hpp"
#include "mwx_periph_timer.hpp"

void pinMode(uint8_t u8pin, E_PIN_MODE mode) {
	if (u8pin < mwx::PIN_DIGITAL::COUNT_DIO) { // DIO0-19
		uint32_t b = 1UL << u8pin;
		switch (mode) {
		case INPUT:
			vAHI_DioSetDirection(b, 0); // input
			vAHI_DioSetPullup(0, b); // disable pull up
			break;

		case OUTPUT:
			vAHI_DioSetDirection(0, b); // output
			vAHI_DioSetPullup(0, b); // disable pull up
			break;

		case OUTPUT_INIT_HIGH:
			vAHI_DioSetOutput(b, 0); // set as HIGH (as default)
			vAHI_DioSetPullup(0, b); // disable pull up
			vAHI_DioSetDirection(0, b); // output
			break;

		case OUTPUT_INIT_LOW:
			vAHI_DioSetOutput(0, b); // set as LOW (as default)
			vAHI_DioSetPullup(0, b); // disable pull up
			vAHI_DioSetDirection(0, b); // output
			break;

		case WAKE_FALLING_PULLUP:
			(void)u32AHI_DioInterruptStatus(); // clear interrupt status
			vAHI_DioSetDirection(b, 0); // set as input
			vAHI_DioSetPullup(b, 0); // enable pull up
			vAHI_DioWakeEnable(b, 0); // also use as DIO WAKE SOURCE
			vAHI_DioWakeEdge(0, b); // rising
			break;

		case WAKE_RISING_PULLUP:
			(void)u32AHI_DioInterruptStatus();
			vAHI_DioSetDirection(b, 0); // set as input
			vAHI_DioSetPullup(b, 0); // enable pull up
			vAHI_DioWakeEnable(b, 0); // also use as DIO WAKE SOURCE
			vAHI_DioWakeEdge(b, 0); // rising
			break;

		case WAKE_FALLING:
			(void)u32AHI_DioInterruptStatus();
			vAHI_DioSetDirection(b, 0); // set as input
			vAHI_DioSetPullup(0, b); // disable pull up
			vAHI_DioWakeEnable(b, 0); // also use as DIO WAKE SOURCE
			vAHI_DioWakeEdge(0, b); // rising
			break;

		case WAKE_RISING:
			(void)u32AHI_DioInterruptStatus();
			vAHI_DioSetDirection(b, 0); // set as input
			vAHI_DioSetPullup(0, b); // disable pull up
			vAHI_DioWakeEnable(b, 0); // also use as DIO WAKE SOURCE
			vAHI_DioWakeEdge(b, 0); // rising
			break;

		case INPUT_PULLUP:
		default:
			vAHI_DioSetDirection(b, 0); // input
			vAHI_DioSetPullup(b, 0); // enable pull up
			break;
		}
	} else
	if (u8pin == 0x80 || u8pin == 0x81) { // DO0/DO1
		uint32_t b = 1UL << (u8pin & 0x7F);

		switch (mode) {
		case OUTPUT:
			bAHI_DoEnableOutputs(true); // output both ports
			vAHI_DoSetPullup(0, b); // disable pull up
			break;

		case OUTPUT_INIT_HIGH:
			bAHI_DoEnableOutputs(true); // set output (both DO/D1) 
			vAHI_DoSetPullup(0, b); // disable pull up
			vAHI_DoSetDataOut(b, 0); // set high
			break;

		case OUTPUT_INIT_LOW:
			bAHI_DoEnableOutputs(true); // set output (both DO/D1)
			vAHI_DoSetPullup(0, b); // disable pull up
			vAHI_DoSetDataOut(0, b); // set low
			break;

		case DISABLE_OUTPUT:
		default:
			bAHI_DoEnableOutputs(false); // disable output
			break;
		}
	}
}

// function ptr table for each DIO interrupt

// set interrupt handler DIO
void attachIntDio(uint8_t u8pin, E_PIN_INT_MODE eMode) {
	uint32_t bm = (1UL << u8pin);
	if (u8pin >= mwx::PIN_DIGITAL::COUNT_DIO) {
		return;
	}

	vAHI_DioInterruptEnable(bm, 0);

	if (eMode == PIN_INT_MODE::RISING) {
		vAHI_DioInterruptEdge(bm, 0);
	}
	else if (eMode == PIN_INT_MODE::FALLING) { // || eMode == PIN_INT_MODE::CHANGE) {
		vAHI_DioInterruptEdge(0, bm);
	}
}

// unset interrupt handler for DIO
void detachIntDio(uint8_t u8pin) {
	uint32_t bm = (1UL << u8pin);

	vAHI_DioInterruptEnable(0, bm);
}

// timer 
uint8_t _mwx_periph_u8_grain_gpio = 0x3;
const uint8_t mwx::periph_timer::_au8_timer_ids[mwx::periph_timer::NUM_TIMERS] = {
		E_AHI_DEVICE_TIMER0, E_AHI_DEVICE_TIMER1,
		E_AHI_DEVICE_TIMER2, E_AHI_DEVICE_TIMER3,
		E_AHI_DEVICE_TIMER4 };
