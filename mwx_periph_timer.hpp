/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdint>
#include <cstring>

#include <jendefs.h>
#include <AppHardwareApi.h>

#include "mwx_debug.h"
#include "mwx_hardware.hpp"
#include "mwx_utils.hpp"
#include "mwx_periph.hpp"

#include "twetimers.h"

extern uint8_t _mwx_periph_u8_grain_gpio;

namespace mwx { inline namespace L1 {

	class periph_ticktimer {
	public:
		periph_ticktimer() {}

		inline bool available() {
			return (_periph_availmap & _PERIPH_AVAIL_TICKTIMER) ? true : false;
		}
	};

	class periph_timer {
		uint8_t _u8_timer;
		uint8_t _u8_state;
		tsTimerContext* _psTimer;

	private:
		periph_timer(const periph_timer&) = delete;
		periph_timer& operator =(const periph_timer&) = delete;
		inline bool _begun() { return _psTimer != nullptr;  }

	public:
		static const uint8_t NUM_TIMERS = 5;
		static const uint8_t TIMER_0 = 0;
		static const uint8_t TIMER_1 = 1;
		static const uint8_t TIMER_2 = 2;
		static const uint8_t TIMER_3 = 3;
		static const uint8_t TIMER_4 = 4;
		static const uint8_t TIMER_UNDEF = 0xFF;

		static const uint8_t _au8_timer_ids[NUM_TIMERS]; 

		static const uint8_t STATE_STARTED_MASK = 0x01;
		static const uint8_t STATE_WAKEUP_ON_START_MASK = 0x02;

	public:
		periph_timer() : _u8_timer(TIMER_UNDEF), _psTimer(nullptr) { }

		periph_timer(uint8_t timer) : _u8_timer(timer), _psTimer(nullptr) { }

		void setup() {
			if (_psTimer == nullptr && _u8_timer != TIMER_UNDEF) {
				_psTimer = new tsTimerContext;
				memset(_psTimer, 0, sizeof(tsTimerContext));
			}
		}

		void begin(uint16_t u16Hz, bool b_sw_int = true, bool b_pwm_out = false) {
			if (_psTimer == nullptr) {
				setup();
			}

			_psTimer->u8Device = _au8_timer_ids[_u8_timer];

			_psTimer->u16Hz = u16Hz;
			get_good_prescale();

			// MWX_DebugMsg(0, "{%d Timer=%d hz=%d pr=%d}", _u8_timer, _psTimer->u8Device, _psTimer->u16Hz, _psTimer->u8PreScale);
			
			if (b_pwm_out) {
				// unset the bit
				switch (_psTimer->u8Device) {
				case E_AHI_DEVICE_TIMER0:
					_mwx_periph_u8_grain_gpio &= ~(1 << 2);
					break;
				case E_AHI_DEVICE_TIMER1:
					_mwx_periph_u8_grain_gpio &= ~(1 << 3);
					break;
				case E_AHI_DEVICE_TIMER2:
					_mwx_periph_u8_grain_gpio &= ~(1 << 4);
					break;
				case E_AHI_DEVICE_TIMER3:
					_mwx_periph_u8_grain_gpio &= ~(1 << 5);
					break;
				case E_AHI_DEVICE_TIMER4:
					_mwx_periph_u8_grain_gpio &= ~(1 << 6);
					break;
				}
			}
			else {
				// set the bit
				switch (_psTimer->u8Device) {
				case E_AHI_DEVICE_TIMER0:
					_mwx_periph_u8_grain_gpio |= (1 << 2);
					break;
				case E_AHI_DEVICE_TIMER1:
					_mwx_periph_u8_grain_gpio |= (1 << 3);
					break;
				case E_AHI_DEVICE_TIMER2:
					_mwx_periph_u8_grain_gpio |= (1 << 4);
					break;
				case E_AHI_DEVICE_TIMER3:
					_mwx_periph_u8_grain_gpio |= (1 << 5);
					break;
				case E_AHI_DEVICE_TIMER4:
					_mwx_periph_u8_grain_gpio |= (1 << 6);
					break;
				}
			}

			_psTimer->bDisableInt = b_sw_int ? 0 : 1;
			_psTimer->bPWMout = b_pwm_out ? 1 : 0;
			_psTimer->u16duty = 512;

			_start();

			return;
		}

		void end() {
			if (!_begun()) return;

			_stop();

			return;
		}

		tsTimerContext* get_timer_context() {
			return _psTimer;
		}

		inline bool available() {
			return (_periph_availmap & (1UL << _u8_timer)) ? true : false;
		}


		/**
		 * @fn	void periph_timer::_on_sleep()
		 *
		 * @brief	Called before sleeping.
		 *
		 */
		void _on_sleep() {
			if (!_begun()) return;

			if (_u8_state & STATE_STARTED_MASK) {
				_u8_state |= STATE_WAKEUP_ON_START_MASK;
			}

			_stop();
		}

		/**
		 * @fn	void periph_timer::_on_sleep()
		 *
		 * @brief	Called when waking up.
		 * 			- Re-initialize device.
		 *
		 */
		void _on_wakeup() {
			if (_begun()) {
				if (_u8_state & STATE_WAKEUP_ON_START_MASK) {
					_u8_state &= ~STATE_WAKEUP_ON_START_MASK;

					_start();
				}
			}
		}

		void change_duty(uint16_t duty, uint16_t duty_max = 1024) {
			if (!_begun()) return;

			_psTimer->u16duty = duty;
			_psTimer->u16duty_max = duty_max;
			vTimerChangeDuty(_psTimer);
		}
		
		void change_hz(uint16_t hz, uint16_t mil = 0) {
			if (!_begun()) return;

			vTimerChangeHzEx(_psTimer, hz*1000 + mil);
		}

	private:
		void get_good_prescale() {
			if (!_begun()) return;

			uint32_t ct = mwx::HARDWARE_IO_CLOCK / _psTimer->u16Hz;
			int ps = 0;
			while (ct > 32767) { // offcourse, can be 65536
				ps++;
				ct /= 2;
			}
			_psTimer->u8PreScale = (uint8_t)ps;
		}

		void _start() {
			vAHI_TimerFineGrainDIOControl(_mwx_periph_u8_grain_gpio);

			vTimerConfig(_psTimer);
			vTimerStart(_psTimer);

			_u8_state |= STATE_STARTED_MASK; // started
			// MWX_DebugMsg(0, "{Timer Start : %d}", _sTimer.u16ct_total);
			return;
		}

		void _stop() {
			vTimerStop(_psTimer);
			vTimerDisable(_psTimer);

			_u8_state &= ~STATE_STARTED_MASK; // stopped

			return;
		}
	};
}}
