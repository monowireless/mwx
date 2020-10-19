/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "mwx_stdio.h"

/// implement millis()
#if defined(_MSC_VER) || defined(__MINGW32__)
#include "windows.h"
# pragma comment(lib, "secur32.lib")
# pragma comment(lib, "winmm.lib")
# pragma comment(lib, "dmoguids.lib")
# pragma comment(lib, "wmcodecdspuuid.lib")
# pragma comment(lib, "msdmo.lib")
# pragma comment(lib, "Strmiids.lib")
#elif defined(__APPLE__) || defined(__linux)
#include <sys/time.h>
#endif

// global uint32_t counter
volatile uint32_t u32TickCount_ms;

// set true to exit main loop.
bool _b_exit_app = false;
int _n_exit_code = 0;

// the Serial
serial_con Serial;

// get system timer in milli seconds
uint32_t millis() {
#if defined(_MSC_VER) || defined(__MINGW32__)
	return (uint32_t)timeGetTime();
#elif defined(__APPLE__) || defined(__linux)
	timeval time;
	gettimeofday(&time, NULL);
	long ms = (time.tv_sec * 1000) + (time.tv_usec / 1000);
	return (uint32_t)ms;
#else
	# warning "no u32GetTick_ms() implementation."
	return 0;
#endif
}

// called when fatal error
void MWX_Panic(uint32_t u32val, const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "\r\nPANIC(%X): ", u32val);
    va_list va2;
    va_copy(va2, va);
    vfprintf(stderr, fmt, va);
    va_end(va2);
	va_end(va);

    // while(1);
}

// for _printf
void _putchar(char c) {
	Serial << c;
}

// function call to exit the app
void mwx_exit(int n) {
    _b_exit_app = true;
    _n_exit_code = n;
}

// the main
int main() {
    u32TickCount_ms = millis();
    setup();
    
    while(1) {
        loop();
        if (_b_exit_app) break;

#if defined(__APPLE__) || defined(__linux)
        usleep(1000); // make a small sleep
#elif defined(_MSC_VER) || defined(__MINGW32__)
        Sleep(1);
#endif
        u32TickCount_ms = millis(); // update internal counter
    }

    return _n_exit_code;
}