#pragma once

/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include <cstring>
#include <cstdint>
#include <climits>

#include <jendefs.h>
#include <AppHardwareApi.h>

#include "mwx_common.hpp"
#include "mwx_debug.h"

namespace mwx { inline namespace L1 {
	namespace PIN_ANALOGUE {
		const uint8_t A1 = 0;
		const uint8_t A2 = 1;
		const uint8_t D0 = 2;
		const uint8_t A3 = 2;
		const uint8_t D1 = 3;
		const uint8_t A4 = 3;
		const uint8_t VCC = 4;

		const uint8_t COUNT_ADC = 5;
	}

	const uint8_t _ADC_PIN_TO_AHI_DEF[PIN_ANALOGUE::COUNT_ADC] = {
		E_AHI_ADC_SRC_ADC_1,
		E_AHI_ADC_SRC_ADC_2,
		E_AHI_ADC_SRC_ADC_3,
		E_AHI_ADC_SRC_ADC_4,
		E_AHI_ADC_SRC_VOLT
	};

	const int16_t ADC_VAL_NOT_YET = SHRT_MIN;

	class periph_analogue {
	private:
		static const uint8_t MASK_RUN = 0x01;
		static const uint8_t MASK_INIT = 0x02;
		static const uint8_t MASK_INIT_RESUME = 0x10;
		static const uint8_t MASK_RUN_RESUME = 0x20;
		static const uint8_t MASK_CAPTURED = 0x80;

	private:
		uint8_t _u8status;
		uint8_t _bmPorts;
		uint8_t _adc_ch;
		uint8_t _kick_ev; // kind of event source calling tick()

		uint8_t _u8tick_every; // tick number to start adc driven by tick(), set 1 or above.
		uint8_t _u8tick_counter; // decresing counter, if reaches zero, kick adc.

		void (*_pf_on_finish)(); // on the end of adc, this function will be called from interrupt handler

		int16_t _i16Adc[PIN_ANALOGUE::COUNT_ADC];

	public:
		static const uint8_t KICK_BY_TICKTIMER = E_AHI_DEVICE_TICK_TIMER;
		static const uint8_t KICK_BY_TIMER0 = E_AHI_DEVICE_TIMER0;
		static const uint8_t KICK_BY_TIMER1 = E_AHI_DEVICE_TIMER1;
		static const uint8_t KICK_BY_TIMER2 = E_AHI_DEVICE_TIMER2;
		static const uint8_t KICK_BY_TIMER3 = E_AHI_DEVICE_TIMER3;
		static const uint8_t KICK_BY_TIMER4 = E_AHI_DEVICE_TIMER4;

	public:

		/**
		 * @fn	periph_analogue::periph_analogue()
		 *
		 * @brief	Default constructor
		 *
		 */
		periph_analogue() : _u8status(0), _bmPorts(0), _adc_ch(0), _kick_ev(0), 
							_u8tick_every(0), _u8tick_counter(0), _pf_on_finish(nullptr) {

			_i16Adc[0] = ADC_VAL_NOT_YET; // dummy for warning removal
			for (int i = 1; i < PIN_ANALOGUE::COUNT_ADC; i++) {
				_i16Adc[i] = ADC_VAL_NOT_YET;
			}
		}

		/**
		 * @fn	void periph_analogue::setup(bool bWaitInit = false, uint8_t capt_tick = 0)
		 *
		 * @brief	Setup the ADC peripherals, ready to start capture.
		 * 			- it needs some time to stabilize ADC Regulator.  
		 * 			- it should be called at setup() procedure.
		 *
		 * @param	bWaitInit	(Optional) True to wait initialize.
		 * @param	capt_tick	(Optional) The capt tick count.
		 * 						           Kick the ADC when the tick() method called `capt_tick' times.
		 * @param   kick_ev     (Optional) Kicking Event (0xFF: TickTimer, 0:Timer0, 1:Timer1, ...)
		 */
		void setup(bool bWaitInit = false, uint8_t kick_ev = E_AHI_DEVICE_TICK_TIMER, void (*pf_on_finish)() = nullptr) {
			// MWX_DebugMsg(0, "Analogue::setup()\r\n");

			if (!bAHI_APRegulatorEnabled()) {
				vAHI_ApConfigure(E_AHI_AP_REGULATOR_ENABLE,
					E_AHI_AP_INT_ENABLE,
					E_AHI_AP_SAMPLE_2,
					E_AHI_AP_CLOCKDIV_500KHZ,
					E_AHI_AP_INTREF);
			}

			// if wants to immediate start of ADC capture, needs polling wait.
			if (bWaitInit) {
				while (!bAHI_APRegulatorEnabled());
			}

			// register an interrupt callback
			vAHI_APRegisterCallback(&ADC_handler);

			// store app call backs
			if (pf_on_finish) {
				_pf_on_finish = pf_on_finish;
			}

			// set scaler (set non zero)
			if (_u8tick_every == 0) _u8tick_every = 1;
			if (_u8tick_counter == 0) _u8tick_counter = 1;

			// kick event
			_kick_ev = kick_ev;
			
			// set flag
			_u8status |= MASK_INIT;
		}

		/**
		 * @fn	void periph_analogue::begin()
		 *
		 * @brief	Start ADC capture.
		 * 			with no parameter, resume previous settings or
		 * 			ADC1/ADC2 capture every TickTimer as default.
		 */
		void begin() {
			// MWX_DebugMsg(0, "Analogue::begin()\r\n");

			_u8status &= 0x0F; // clear higher bits (for FIRST_CAPTURE mask)

			// clear capture data
			for (int i = 0; i < PIN_ANALOGUE::COUNT_ADC; i++) {
				_i16Adc[i] = ADC_VAL_NOT_YET;
			}

			// if ports are not set, use A1,A2 as default.
			if (_bmPorts == 0) {
				_bmPorts = (1 << PIN_ANALOGUE::A1) | (1 << PIN_ANALOGUE::A2);
			}

			// if not setup, prepare ADC circuit.
			if (!(_u8status & MASK_INIT)) {
				setup(true);
			}

			// kick the first ADC
			_u8status |= MASK_RUN;
			_adc_start_newly();			
		}

		/**
		 * @fn	void periph_analogue::begin(uint8_t bmPorts)
		 *
		 * @brief	Begins the given bm ports
		 *
		 * @param	bmPorts	The ADC ports.
		 */
		void begin(uint8_t bmPorts, uint8_t capt_tick = 1) {
			// set scaler
			if (capt_tick == 0) capt_tick = 1;
			_u8tick_every = capt_tick;
			_u8tick_counter = capt_tick;

			// set ports
			_bmPorts = bmPorts;
			begin();
		}

		void end() {
			vAHI_ApConfigure(E_AHI_AP_REGULATOR_DISABLE,
				E_AHI_AP_INT_DISABLE,
				E_AHI_AP_SAMPLE_2,
				E_AHI_AP_CLOCKDIV_500KHZ,
				E_AHI_AP_INTREF);

			_u8status &= 0xF0; // clear current status. running status is saved on higher bits.
		}

		/**
		 * @fn	inline int16_t periph_analogue::read(uint8_t s)
		 *
		 * @brief	Reads the captured ADC value as given s.
		 *
		 * @param	s	The ADC port number.
		 *
		 * @returns	ADC value in mV
		 * 			ADC_VAL_NOT_YET, if not captured 
		 */
		inline int16_t read(uint8_t s) {
			int32_t v = ADC_VAL_NOT_YET;

			if (s < PIN_ANALOGUE::COUNT_ADC) {
				if (_i16Adc[s] != ADC_VAL_NOT_YET)
					v = _i16Adc[s];
			}

			if (s < 4) { // ADC1..4
				v = v * 2470 / 1024;
			}
			else if (s == 4) {
				v = v * 3705 >> 10;
			}

			return (int16_t)v;
		}

		/**
		 * @fn	inline int16_t periph_analogue::read_raw(uint8_t s)
		 *
		 * @brief	Reads the captured ADC value as given s.
		 *
		 * @param	s	The ADC port number.
		 *
		 * @returns	raw adc value, or
		 * 			ADC_VAL_NOT_YET, if not captured 
		 */
		inline int16_t read_raw(uint8_t s) {
			return (s < PIN_ANALOGUE::COUNT_ADC) ? _i16Adc[s] : ADC_VAL_NOT_YET;
		}

		/**
		 * @fn	inline bool periph_analogue::available()
		 *
		 * @brief	Check if ADC capture is finished.
		 * 			The captured flag is unset upon this call.
		 *
		 * @returns	True if the capture is completed, false if it's not yet.
		 */
		inline bool available() {
			if (_u8status & MASK_CAPTURED) {
				_u8status &= ~MASK_CAPTURED;
				return true;
			}
			else return false;
		}

		/**
		 * @fn	inline void periph_analogue::_adc_capt()
		 *
		 * @brief	Capture ADC value and run next port.
		 * 			- called from ADC_handler() ADC interrupt handler.
		 *
		 */
		inline void _adc_capt() {
			_i16Adc[_adc_ch] = (int16)u16AHI_AdcRead();

			_adc_ch++;
			_adc_start();
		}

		/**
		 * @fn	inline void periph_analogue::_adc_start()
		 *
		 * @brief	Start ADC capture
		 * 			note: on finishing ADC capture, ADC_handler() will be called.
		 */
		inline void _adc_start() {
			while (_is_running()) {
				uint8_t mask = _bmPorts >> _adc_ch;

				if (mask == 0) {
					_u8status |= MASK_CAPTURED;

					// MWX_DebugMsg(0, "C");
					// on finish, call user handler.
					if (_pf_on_finish != nullptr) {
						(*_pf_on_finish)();
						// MWX_DebugMsg(0, "*");
					}
					break; // finish
				}

				// start ADC with _adc_ch
				if (mask & 0x01) {
					// MWX_DebugMsg(0, "<A:%d>", _adc_ch);

					vAHI_AdcEnable(
						E_AHI_ADC_SINGLE_SHOT,
						E_AHI_AP_INPUT_RANGE_2,
						_ADC_PIN_TO_AHI_DEF[_adc_ch]);
					vAHI_AdcStartSample();
					break;
				}
				else {
					++_adc_ch;
				}
			}
		}

		/**
		 * @fn	inline bool periph_analogue::_is_running()
		 *
		 * @brief	if running, return true.
		 *
		 * @returns	True if running, false if not.
		 */
		inline bool _is_running() {
			return (_u8status & MASK_RUN);
		}

		/**
		 * @fn	inline void periph_analogue::tick()
		 *
		 * @brief	Start capture.
		 *
		 */
		inline void tick() {
			_adc_start_newly();
		}

		/**
		 * @fn	void periph_analogue::_on_sleep()
		 *
		 * @brief	Executes before the sleep action
		 * 			- save the ADC status (initialized/running)  
		 * 			- stop the ADC circuit.
		 *
		 */
		void _on_sleep() {
			_u8status <<= 4;
			end();
		}

		/**
		 * @fn	void periph_analogue::_on_wakeup()
		 *
		 * @brief	Executes when the wakeup action
		 * 			- restart ADC capturing, 
		 */
		void _on_wakeup() {
			if (_u8status != 0) {
				if (_u8status & MASK_INIT_RESUME) {
					setup(true);
				}

				if (_u8status & MASK_RUN_RESUME) {
					_u8tick_counter = _u8tick_every; // set resume counter

					begin(); // restart again
				}
			}

			_u8status &= 0x0F; // clear hihger bits (resume bit and first capture flag)
		}

		/**
		 * @fn	static void periph_analogue::ADC_handler(uint32 u32Device, uint32 u32ItemBitmap);
		 *
		 * @brief	Handler, called when the ADC
		 *
		 * @param	u32Device	 	The device. (see AHI manual)
		 * @param	u32ItemBitmap	The item bitmap. (see AHI manual)
		 */
		static void ADC_handler(uint32 u32Device, uint32 u32ItemBitmap);

		/**
		 * @fn	inline void periph_analogue::_adc_event_source(uint8_t id)
		 *
		 * @brief	check if matching event source.
		 */
		inline bool _adc_event_source(uint8_t id) {
			if (_kick_ev == id) {
				if (_is_running()) {
					if (--_u8tick_counter == 0) {
						_u8tick_counter = _u8tick_every;
						return true; // then, will be called tick().
					}
				}
			}

			return false;
		}

	private:
		/**
		 * @fn	inline void periph_analogue::_adc_start_newly()
		 *
		 * @brief	Start capture.
		 */
		inline void _adc_start_newly() {
			_adc_ch = 0;
			_adc_start();
		}
	};
}}

/** @brief	The global ADC object. */
extern mwx::periph_analogue Analogue;

/**
 * @fn	static inline int16_t analogRead(uint8_t s)
 *
 * @brief	Read ADC value (0..1023)
 *
 * @param	s	ADC pin
 *
 * @returns	ADC value (0..1023)
 * 			ADC_VAL_NOT_YET, if not captured.
 */
static inline int16_t analogRead(uint8_t s) {
	return Analogue.read_raw(s);
}

/**
 * @fn	static inline int16_t analogRead_mv(uint8_t s)
 *
 * @brief	Read ADC value in [mV].
 *
 * @param	s	ADC pin
 *
 * @returns	ADC value in [mV].
 * 			ADC_VAL_NOT_YET, if not captured.
 */
static inline int16_t analogRead_mv(uint8_t s) {
	return Analogue.read(s);
}
