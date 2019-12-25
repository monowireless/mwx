/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdarg>
#include <cstdint>

#include <utility>
#include <initializer_list>
#include <algorithm>


#include <jendefs.h>
#include <AppHardwareApi.h>

#include "mwx_debug.h"
#define DEBUG_SPI_LEVEL 99

#include "mwx_utils_smplque.hpp"
#include "mwx_stream.hpp"

#include "mwx_hardware.hpp"

namespace mwx { inline namespace L1 {
	namespace SPI_CONF {
		const uint8_t MSBFIRST = 0;
		const uint8_t LSBFIRST = 1;
		const uint8_t SPI_MODE0 = 0;
		const uint8_t SPI_MODE1 = 1;
		const uint8_t SPI_MODE2 = 2;
		const uint8_t SPI_MODE3 = 3;
	};

	struct SPISettings {
		uint32_t _clock;
		uint8_t _bitOrder;
		uint8_t _dataMode;
		SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) :
			_clock(clock), _bitOrder(bitOrder), _dataMode(dataMode) { }
	};

	class periph_spi {
		uint8_t _div;
		uint8_t _dataMode;
		uint8_t _bitOrder;
		uint8_t _slave_select;

		// why this method is there???
		void begin(uint8_t slave_select) {
			// set slave count
			_slave_select = slave_select;
			if (_slave_select >= 3) {
				_slave_select = 2;
			}
		}

	public:
		periph_spi() : _div(0), _dataMode(0), _bitOrder(0) {}

		/**
		 * @fn	void SPIClass::begin(uint8_t slave_count)
		 *
		 * @brief	Begins the given slave id (0 .. 2).
		 * 			0 : DIO19  
		 * 			1 : DIO0  (DIO19 is allocated for SPI select pin)
		 * 			2 : DIO1  (DIO19/DIO0 are allocated for SPI select pin)
		 *
		 * @param	slave_count	Number of slaves.
		 */

		void begin(uint8_t slave_select, SPISettings settings) {
			begin(slave_select);

			_div = _calc_clock_div(settings._clock);
			_bitOrder = settings._bitOrder;
			_dataMode = settings._dataMode;

			vAHI_SpiConfigure(
				_slave_select,
				_bitOrder, // bitorder 1 to LSBfirst
				_dataMode & 2 ? 1 : 0, // polarity
				_dataMode & 1 ? 1 : 0, // phase
				_div, // clock divisor
				FALSE, // int enabled
				FALSE // AUTO slave select pin
			);

			MWX_DebugMsg(DEBUG_SPI_LEVEL, "<SPI SL=%d div=%d %d %d>\r\n", _slave_select, _div, _bitOrder, _dataMode);
		}

		// proceed with last configuration
		void beginTransaction() {
			vAHI_SpiSelect(1 << _slave_select);
		}

		// start with parameter settings
		void beginTransaction(SPISettings settings) {
			begin(_slave_select, settings);
			beginTransaction();
		}

		// stop transmission (clear chip select pin)
		void endTransaction() {
			vAHI_SpiStop();
		}

		// disables SPI Master
		void end() {
			vAHI_SpiDisable();
		}

		inline uint8_t transfer(uint8_t data) {
			// vAHI_SpiStartTransfer8(data);
			vAHI_SpiStartTransfer(7, data);
			vAHI_SpiWaitBusy();
			uint8_t c = u8AHI_SpiReadTransfer8();
			MWX_DebugMsg(DEBUG_SPI_LEVEL, "[%02X->%02X]", data, c);
			return c;
		}

		inline uint16_t transfer16(uint16_t data) {
			vAHI_SpiStartTransfer16(data);
			vAHI_SpiWaitBusy();
			return u16AHI_SpiReadTransfer16();
		}

		inline uint32_t transfer32(uint32_t data) {
			vAHI_SpiStartTransfer32(data);
			vAHI_SpiWaitBusy();
			return u32AHI_SpiReadTransfer32();
		}

		void transfer(void* buf, size_t count) {
			if (buf != nullptr) {
				uint8_t* p = static_cast<uint8_t*>(buf);

				while (count-- != 0) {
					transfer(*p++);
				}
			}
		}


	private:
		/**
		 * @fn	uint8_t SPIClass::_calc_clock_div(uint32_t u32clock)
		 *
		 * @brief	Calculates the clock div
		 *
		 * @param	u32clock	The 32clock.
		 *
		 * @returns	The calculated clock div.
		 */
		uint8_t _calc_clock_div(uint32_t u32clock) {
			if (u32clock == 0) return 0;
			uint32_t div = (8000000UL * 128 / u32clock + 50) / 128; // clk = 16M / (2*div), 50 is adjust an effect of division.
			return uint8_t(div);
		}

	public:
		class transceiver : public mwx::stream<transceiver> {
		public:
			typedef uint8_t size_type;
			typedef uint8_t value_type;

			friend class mwx::stream<transceiver>;
			using SUPER = mwx::stream<transceiver>;

		private:
			// disable copy constructor and assign operator
			transceiver& operator= (const transceiver& ref) = delete;
			transceiver(const transceiver& ref) = delete;

			periph_spi& _spi;
			mwx::smplque<uint8_t, mwx::alloc_local<uint8_t,16>> _que;

		public:
			// defines move constructor instead, but may not be used, ROV works fine.
			transceiver(transceiver&& ref) : _spi(ref._spi) {
				// preventing calling destructor by ref.
			}

		public:
			transceiver(periph_spi& ref) : _spi(ref), _que() {
				SUPER::pvOutputContext = (void*)this;
				_spi.beginTransaction();
			}

			~transceiver() {
				_spi.endTransaction();
			}

			operator bool() {
				return true; // false when an error occurred during initialization.
			}

			// transfer a byte
			uint8_t transfer(uint8_t val) {
				return _spi.transfer(val);
			}

			uint16_t transfer16(uint16_t val) {
				return _spi.transfer16(val);
			}

			uint32_t transfer32(uint32_t val) {
				return _spi.transfer32(val);
			}

			// write bytes (discard read values)
			size_type operator() (const value_type* p_data, size_type quantity) { 
				size_type n = quantity;
				while (n--) write(*p_data++);
				return quantity;
			}
			// write bytes (discard read values)
			template <typename T, int S> inline size_type operator() (const T(&ary)[S]) { 
				for (auto x : ary) write(x);
				return S;
			}
			// write bytes (discard read values)
			size_type operator() (std::initializer_list<const value_type>&& list) {
				for (auto x : list) write(x);
				return list.size();
			}

			// overload/override the operator <<
			// using SUPER::operator<<;

			transceiver& operator << (uint16_t v) {
				// call 16bit transfer
				uint16_t r = _spi.transfer16(v);

				MWX_DebugMsg(DEBUG_SPI_LEVEL, "[%04X->%04X]", v, r);

				// store return value (as bigendian order)
				_que.push(r >> 8);
				_que.push(r & 0xFF);

				return *this;
			}
			
			transceiver& operator << (uint32_t v) {
				// call 32bit transfer.
				uint32_t r = _spi.transfer32(v);

				// store return value (as bigendian order)
				_que.push(r >> 24);
				_que.push((r >> 16) & 0xFF);
				_que.push((r >> 8) & 0xFF);
				_que.push(r & 0xFF);

				return *this;
			}

			// override int type (default output is printf("%d", value) style, but it should assume byte input here.
			transceiver& operator << (int v) {
				value_type c = _spi.transfer(v & 0xFF);
				_que.push(c);
				return *this;
			}
			transceiver& operator << (const char* msg) {
				SUPER::operator<<(msg);
				return *this;
			}
			transceiver& operator << (const uint8_t* msg)  {
				SUPER::operator<<(msg);
				return *this;
			}
			transceiver& operator << (char c)  {
				SUPER::operator<<(c);
				return *this;
			}
			transceiver& operator << (uint8_t c) {
				SUPER::operator<<(c);
				return *this;
			}
			transceiver& operator << (mwx::mwx_format&& prt) {
				SUPER::operator<<(std::forward<mwx::mwx_format&&>(prt));
				return *this;
			}
			template <typename T>
			transceiver& operator << (mwx::bigendian<T>&& v) {
				SUPER::operator<<(std::forward<mwx::bigendian<T>&&>(v));
				return *this;
			}
			template <typename T>
			transceiver& operator << (std::pair<T*, T*>&& t) {
				SUPER::operator<<(std::forward<std::pair<T*, T*>&&>(t));
				return *this;
			}
			template <typename T>
			transceiver& operator << (std::pair<T*, int>&& t) {
				SUPER::operator<<(std::forward<std::pair<T*, int>&&>(t));
				return *this;
			}
			transceiver& operator << (bytelist&& bl) {
				SUPER::operator<<(std::forward<bytelist&&>(bl));
				return *this;
			}

		public: // stream interfaceF
			inline void flush(void) {
				; // no buffered operation here.
			}

			inline int read() {
				if (_que.empty()) return -1;
				else {
					value_type c = _que.front();
					_que.pop();
					return c;
				}
			}

			inline size_t write(int n) {
				value_type c = _spi.transfer(n & 0xFF);
				_que.push(c);

				return 1;
			}

			// for upper class use
			static void vOutput(char out, void* vp) {
				transceiver* p_wire = (transceiver*)vp;
				if (p_wire != nullptr) {
					p_wire->write(out);
				}
			}
		};

		transceiver get_rwer() {
			return transceiver(*this);
		}

	private:
		friend class transceiver;
	};
}}
