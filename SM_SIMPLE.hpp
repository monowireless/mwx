#pragma once

#include <cstdint>
#include <cstring>
#include "mwx_periph.hpp"

// very simple class to control state used in loop().
template <typename STATE>
class SM_SIMPLE {
	uint32_t _u32_flag_value;  // optional data when flag is set.
	uint32_t _ms_start;		// system time when start waiting.
	uint32_t _ms_timeout;	// timeout duration

	STATE _step;			// current state
	STATE _step_prev;		// previous state
	bool_t _b_flag; 		// flag control.
public:
	// init
	void setup() { memset(this, 0, sizeof(SM_SIMPLE)); }
	// call befoer sleeping (save state machine status)
	void on_sleep(bool b_save_state = false) {
		STATE save = _step;
		setup();
		if(b_save_state) _step = _step_prev = save;
	}

	// state control
	void next(STATE next) { _step = next; } // set next state
	STATE state() { return _step; } // state number
	bool b_more_loop() { // if state is changed during the loop, set true
		if (_step != _step_prev) { _step_prev = _step; return true; }
		else return false;
	}

	// timeout control
	void set_timeout(uint32_t timeout) {
		_ms_start = millis();
		_ms_timeout = timeout;
	}
	bool is_timeout() { return (millis() - _ms_start) >= _ms_timeout; }

	// flag control
	void clear_flag() { _b_flag = false; _u32_flag_value = 0; }
	void set_flag(uint32_t u32_flag_value = 0) {
		_b_flag = true;
		_u32_flag_value = u32_flag_value; }
	uint32_t get_flag_value() { return _u32_flag_value; }
	bool is_flag_ready() { return _b_flag; }
};