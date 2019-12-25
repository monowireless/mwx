/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <stdarg.h>
#include "twecommon.h"
#include "tweserial.h"
#include "tweserial_jen.h"
#include "tweprintf.h"

#include "mwx_stream.hpp"

#include <AppHardwareApi.h>
#include "serial.h"

namespace mwx { inline namespace L1 {
	class serial_jen : public mwx::stream<serial_jen> {
		uint8_t* _au8SerialTxBuffer; // internal buffer FIFO queue
		uint8_t* _au8SerialRxBuffer; // internal buffer FIFO queue
		struct {
			uint8_t _u8Port;
			uint8_t _u8Conf;
			uint16_t _u16HectBaud;
		} _serdef;

		uint16_t BUF_TX;
		uint16_t BUF_RX;

		TWE_tsFILE* _psSer;

		// constructor, does nothing
		serial_jen() {}
		
	public:
		using SUPER = mwx::stream<serial_jen>;

		// constructor, does nothing
		serial_jen(uint8_t u8Port) {
			_psSer = nullptr;
			_serdef._u8Port = u8Port;
			_serdef._u16HectBaud = 1152;
			_serdef._u8Conf = 0;

			BUF_TX = 0;
			BUF_RX = 0;
		}

		void setup(uint16_t buf_tx, uint16_t buf_rx) {
			BUF_TX = buf_tx;
			BUF_RX = buf_rx;

			_au8SerialTxBuffer = new uint8_t[BUF_TX];
			_au8SerialRxBuffer = new uint8_t[BUF_RX];

			_psSer = new TWE_tsFILE();
			SUPER::_bSetup = (uint8_t)true;
		}

		void begin(uint32_t speed = 115200, uint8_t config = 0x06) {
			if (_setup_finished()) {
				_serdef._u8Conf = 0;
				_serdef._u16HectBaud = speed / 100;

				TWETERM_tsSerDefs sDef;
				sDef.au8RxBuf = _au8SerialRxBuffer;
				sDef.au8TxBuf = _au8SerialTxBuffer;
				sDef.u16RxBufLen = BUF_RX;
				sDef.u16TxBufLen = BUF_TX;

				sDef.u32Baud = _serdef._u16HectBaud * 100;
				TWETERM_vInitJen(_psSer, _serdef._u8Port, &sDef);

				SUPER::pvOutputContext = (void*)_psSer;
				SUPER::_bReady = 1;
			}
		}

		void end(void) {
			// de-init should be implemented.
			SUPER::_bReady = 0;
		}

		inline int available() {
			return (!SERIAL_bRxQueueEmpty(_serdef._u8Port));
		}

		inline void flush(void) {
			_psSer->fp_flush(_psSer);
		}

		inline int read() {
			int iChar = -1;
			if (!SERIAL_bRxQueueEmpty(_serdef._u8Port)) {
				iChar = SERIAL_i16RxChar(_serdef._u8Port);
			}
			return iChar;
		}

		inline size_t write(int n) {
			return (int)SERIAL_bTxChar(_serdef._u8Port, n);
		}

		// for upper class use
		static void vOutput(char out, void* vp) {
			TWE_tsFILE* fp = (TWE_tsFILE*)vp;
			fp->fp_putc(out, fp);
		}

		// called about to sleep.
		void _on_sleep() {
			if (_setup_finished()) {
				flush();
			}
		}

		// called when waking up.
		void _on_wakeup() {
			if (_setup_finished()) {
				begin();
			}
		}

		// class specific
		TWE_tsFILE* get_tsFile() { return _psSer; }
		uint8 get_Port() { return _serdef._u8Port; }
	};
}}
