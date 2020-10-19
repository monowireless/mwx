/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <stdarg.h>
#include <stdio.h>

#if defined(__APPLE__) || defined(__linux)
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#elif defined(_MSC_VER) || defined(__MINGW32__)
#include <conio.h>
#include <windows.h>
/* serial functions */
static inline int TWETERM_iPutC(int c) { return _putch(c); }
static inline int TWETERM_iGetC() { if (_kbhit()) return _getch(); else return -1; }
#endif

#include "mwx_stream.hpp"
#include "_mwx_stdio.h"

namespace mwx { inline namespace L1 {
	class serial_con : public mwx::stream<serial_con> {
#if defined(__APPLE__) || defined(__linux)
		termios CookedTermIos; // cooked mode
		termios RawTermIos; // raw mode
#endif

		int _c_read;
		bool _begun;

		static int GetChar() {
#if defined(__APPLE__) || defined(__linux)
			return ::getchar();
#elif defined(_MSC_VER) || defined(__MINGW32__)
			return TWETERM_iGetC(); 
#endif
		}

		static int PutChar(int c) {
#if defined(__APPLE__) || defined(__linux)
			::putchar(char(c));
			return 1;
#elif defined(_MSC_VER) || defined(__MINGW32__)
			return TWETERM_iPutC(c); 
#endif
		}

	public:
		using SUPER = mwx::stream<serial_con>;

		serial_con() : _c_read(-1), _begun(false) {
		}

		~serial_con() {
			end();
		}

		// begin() : start UART port
		void begin() {
#if defined(__APPLE__) || defined(__linux)
			// save the intial state of terminal
			::tcgetattr(STDIN_FILENO, &CookedTermIos);

			// create RAW mode terminal
			RawTermIos = CookedTermIos;
			::cfmakeraw(&RawTermIos);
			RawTermIos.c_oflag |= OPOST;
			RawTermIos.c_lflag |= ISIG; // enable SIGINT(Ctrl+C)

			// set stdin as RAW mode 
			::tcsetattr(STDIN_FILENO, TCSANOW, &RawTermIos);
			
			// for nonblocking getchar()
			::fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
#endif
			// set inernal flag
			_begun = true;
		}

		// end() : de-init the port
		void end(void) {
			if (_begun) {
#if defined(__APPLE__) || defined(__linux)
				::tcsetattr(STDIN_FILENO, 0, &CookedTermIos);
#endif
				_begun = false;
			}
		}

		inline int available() {
			if (_c_read >= 0) return true;
			if (_c_read == -1) {
				_c_read = GetChar();
				return (_c_read >= 0);
			}
			return 0;
		}

		inline void flush(void) {
			::fflush(stdout);
		}

		inline int read() {
			if (_c_read >= 0) {
				int c = _c_read;
				_c_read = -1;
				return c;	
			} else {
				int c = GetChar();
				if (feof(stdin)) {
					mwx_exit(0);
					return -1;
				} else {
					return c;
				}
			}
		}

		inline size_t write(int n) {
			PutChar(n);
			return 1;
		}

		// for upper class use
		static void vOutput(char out, void* vp) {
			PutChar(out);
		}

		// called about to sleep.
		void _on_sleep() {
		}

		// called when waking up.
		void _on_wakeup() {
		}

public:
	};
}}
