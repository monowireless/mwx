/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"

namespace mwx { inline namespace L1 {
	class periph_led_timer {
		uint8_t _port;
		uint8_t _mode;
		uint8_t _on;
		uint8_t _opt;

		uint16_t _ct;
		uint16_t _ct_now;

		static const uint8_t _MODE_BLINK = 0;       // internal use
		static const uint8_t _MODE_TIMER = 1;       // internal use
	public:
		static const uint8_t BLINK = 0x00;      // just blink it
		static const uint8_t ONESHOT = 0x11;    // just for led events
		static const uint8_t ON_RX = 0x12;      // light for some time on RX event (put .begin(ON_RX) at receive().)
		static const uint8_t ON_TX_COMP = 0x13; // light for some time on TX comp event (put .begin(ON_RX) at tx_complete().)

		periph_led_timer(uint8_t pt) : _ct(0), _ct_now(0), _mode(0), _on(HIGH), _port(pt), _opt(0) { }

		operator bool() { return _ct_now != 0; }

        // setup parameters
		void setup(uint8_t mode, uint16 ct) {
			_mode = mode >> 4;
			_opt = mode & 0xF;
			_ct = ct;
			_ct_now = 0xFFFF; // in use (0: not used)
		}

		void setup(uint8_t port, uint8_t mode, uint16 ct) {
			_port = port;
			_mode = mode >> 4;
			_opt = mode & 0xF;
			_ct = ct;
			_ct_now = 0xFFFF; // in use (0: not used)
		}

        // start lighting.
        // begin() procedure or on event (opt should set other than 0)
		void begin(uint8_t opt = 0) {
			if ((opt & 0xF) == _opt && _ct_now != 0) {
				_ct_now = _ct;
				_on = LOW;

				digitalWrite(_port, LOW); // LOW as 0
			}
		}

		void end() {
			_ct_now = 0; // no use
		}

        // called at TickTimer event (every msec)
		void tick() {
			if (_ct_now != 0 && _ct_now != 0xFFFF) {
				switch (_mode) {
				case _MODE_BLINK: // blink
					if (--_ct_now == 0) {
						_on = (_on == HIGH) ? LOW : HIGH;
						_ct_now = _ct;

						digitalWrite(_port, _on ? HIGH : LOW); // LOW as 0
					}
					break;

				case _MODE_TIMER: // timer
					if (_ct_now) {
						if (--_ct_now == 0) {
							digitalWrite(_port, HIGH);
							_ct_now = 0xFFFF; // stopped
						}
					}
					break;
				}
			}
		}

		void _on_sleep() {
			if (_ct_now != 0) { // non zero means, in use
				_ct_now = 0xFFFF;
				digitalWrite(_port, HIGH);
			}
		}

		void _on_wakeup() {
			if (_ct_now == 0xFFFF) {
				if (_opt == _MODE_BLINK) {
					begin(BLINK);
				}
			}
		}
	};
}}
