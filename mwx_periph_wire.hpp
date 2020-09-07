/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdarg>
#include <cstdint>

#include <utility>
#include <initializer_list>
#include <algorithm>
#include "mwx_utils_smplque.hpp"

#include <jendefs.h>
#include <AppHardwareApi.h>

#include "mwx_appcore.hpp"
#include "mwx_stream.hpp"
#include "mwx_debug.h"

namespace mwx { inline namespace L1 {
	namespace WIRE_CONF {
		const uint8_t WIRE_50KHZ = 63;
		const uint8_t WIRE_66KHZ = 47;
		const uint8_t WIRE_80KHZ = 39;
		const uint8_t WIRE_100KHZ = 31;
		const uint8_t WIRE_133KHZ = 23;
		const uint8_t WIRE_160KHZ = 19;
		const uint8_t WIRE_200KHZ = 15;
		const uint8_t WIRE_266KHZ = 11;
		const uint8_t WIRE_320KHZ = 9;
		const uint8_t WIRE_400KHZ = 7;

		const uint8_t WIRE_SPEED_MASK = 0x3F;
		const uint8_t WIRE_PORT_ALT_MASK = 0x40;
		const uint8_t WIRE_PORT_INIT_MASK = 0x80;
	}

	template <int SIZ = MWX_TWOWIRE_BUFF>
	class periph_twowire {
		static const int DLVL = 99;
		friend class reader;
		friend class writer;
	public:
		typedef uint8_t size_type;
		typedef uint8_t value_type;

		
	private:
		/** @brief	internal queue to store read bytes. */
		mwx::smplque<value_type, mwx::alloc_local<value_type, SIZ>> _que;

		uint8_t _mode;
		uint8_t _addr;
		uint8_t _last_write;
		uint8_t _speed_and_portalt;


		static const int ADDR_UNSET = 0xFF;
		static const int MODE_TX = 1;
		static const int MODE_RX = 2;
		static const int MODE_NONE = 0;

	public: // defines helper classes
	
		/**
		 * helper class to perform twowire write operation.
		 */
		class writer : public mwx::stream<writer> {
			friend class mwx::stream<writer>;
			using SUPER = mwx::stream<writer>;

			uint8_t _mode;
			uint8_t _devid;
			periph_twowire& _wire;

			// disable copy constructor and assign operator
			writer& operator= (const writer& ref) = delete;
			writer(const writer& ref) = delete;

		public:
			// defines move constructor instead, but may not be used, ROV works fine.
			writer(writer&& ref) : _wire(ref._wire)
			{
				// preventing calling destructor by ref.
				ref._mode = MODE_NONE;
				ref._devid = ADDR_UNSET;

				mwx::stream<writer>::pvOutputContext = (void*)&_wire;
			}

		public:
			writer(periph_twowire& ref, uint8_t devid) : _wire(ref) {
				if (_wire._mode == periph_twowire::MODE_NONE) {
					_wire.beginTransmission(devid);
				}
				_mode = _wire._mode;
			}

			~writer() {
				if (_mode == periph_twowire::MODE_TX) {
					_wire.endTransmission();
				}
			}

			operator bool() {
				return (_wire._mode == periph_twowire::MODE_TX);
			}

			size_type operator() (const value_type val) { return _wire.write(val); }
			size_type operator() (const value_type* p_data, size_type quantity) { return _wire.write(p_data, quantity); }
			template <typename T, int S> inline size_type operator() (const T(&ary)[S]) { return _wire.write<S>(ary); }
			size_type operator() (std::initializer_list<const value_type>&& list) { return _wire.write(list); }

			// override it (default output is printf("%d", value) style, but it should assume byte input here.
			// using SUPER::operator<<;
			writer& operator << (uint16_t v) { return SUPER::operator<< (v); }
			writer& operator << (uint32_t v) { return SUPER::operator<< (v); }
			writer& operator << (int v) {
				_wire.write(uint8_t(v & 0xFF));
				return *this;
			}
			writer& operator << (const char* msg) {
				SUPER::operator<<(msg);
				return *this;
			}
			writer& operator << (const uint8_t* msg)  {
				SUPER::operator<<(msg);
				return *this;
			}
			writer& operator << (char c)  {
				SUPER::operator<<(c);
				return *this;
			}
			writer& operator << (uint8_t c) {
				SUPER::operator<<(c);
				return *this;
			}
			writer& operator << (mwx::mwx_format&& prt) {
				SUPER::operator<<(std::forward<mwx::mwx_format&&>(prt));
				return *this;
			}
			template <typename T>
			writer& operator << (mwx::bigendian<T>&& v) {
				SUPER::operator<<(std::forward<mwx::bigendian<T>&&>(v));
				return *this;
			}
			template <typename T>
			writer& operator << (std::pair<T*, T*>&& t) {
				SUPER::operator<<(std::forward<std::pair<T*, T*>&&>(t));
				return *this;
			}
			template <typename T>
			writer& operator << (std::pair<T*, int>&& t) {
				SUPER::operator<<(std::forward<std::pair<T*, int>&&>(t));
				return *this;
			}
			writer& operator << (bytelist&& bl) {
				SUPER::operator<<(std::forward<bytelist&&>(bl));
				return *this;
			}

		private: // stream interface
			inline void flush(void) {
				; // no buffered operation here.
			}

			inline size_t write(int n) {
				return (size_t)operator() (n);
			}

			// for upper class use
			static void vOutput(char out, void* vp) {
				periph_twowire* p_wire = (periph_twowire*)vp;
				if (p_wire != nullptr) {
					p_wire->write(uint8_t(out));
				}
			}
		};

		/**
		 * helper class to perform twowire read operation.
		 */
		class reader : public mwx::stream<reader> {
			friend class mwx::stream<reader>;

			const uint8_t _read_count;
			uint8_t _read_count_now;

			uint8_t _mode;
			uint8_t _devid;
			periph_twowire& _wire;

			// disable copy constructor and assign operator.
			reader& operator= (const reader& ref) = delete;
			reader(const reader& ref) = delete;

		public:
			// defines move constructor instead, but may not be used, ROV works fine.

			reader(reader&& ref) : _wire(ref._wire), _read_count(ref._read_count), _read_count_now(0)
			{
				// preventing calling destructor by ref.
				ref._mode = MODE_NONE;
				ref._devid = ADDR_UNSET;
			}

		public:

			/**
			 * @fn	reader::reader(reader&& ref, uint8_t readcount = 0)
			 *
			 * @brief	Constructor
			 *
			 * @param [in,out]	ref		 	The reference.
			 * @param 		  	readcount	(Optional) The readcount.
			 * 								  0: no check for reading bytes (no automatic send STOP bit)
			 * 								  other: if reaches `readcount', set STOP bit at the final byte receive.
			 */

			reader(periph_twowire& ptr, uint8_t devid, uint8_t read_count = 0) : _wire(ptr), _read_count(read_count), _read_count_now(0) {
				if (_wire._mode == periph_twowire::MODE_NONE) {
					_wire.beginReceive(devid);
				}
				_mode = _wire._mode;
			}

			~reader() {
				if (_mode == periph_twowire::MODE_RX) {
					bool b_send_dummy_stop = (_read_count && (_read_count == _read_count_now)) ? false : true;
					_wire.endReceive(b_send_dummy_stop);
				}
			}

			operator bool() {
				return (_mode == periph_twowire::MODE_RX);
			}

			inline int operator() (bool b_stop = false) {
				if (_check_readcount(b_stop)) {
					int c = _wire.receive(b_stop);
					return c;
				}
				return -1;
			}

		private:
			// return true, we can read it.
			bool _check_readcount(bool& b_stop) {
				bool b_read_more = true;
				if (_read_count) {
					if (_read_count == _read_count_now) { // reaches planned receiving bytes.
						b_read_more = false;
					} 
					else { // read more
						++_read_count_now;

						if (_read_count == _read_count_now) { // final byte, should send b_stop.
							b_stop = true;
						}
					}
				}
				else {
					b_read_more = true;
				}

				return b_read_more;
			}

		private: // implement stream interface
			inline int read() {
				bool b_stop = false;
				return operator ()(b_stop);
			}
		};
		
	public:
		/**
		 * @fn	periph_twowire::periph_twowire()
		 *
		 * @brief	Default constructor
		 *
		 */
		periph_twowire() : _que(), _addr(ADDR_UNSET), _mode(MODE_NONE), _speed_and_portalt(0) { }

		/**
		 * @fn	void begin(const uint8_t u8mode = WIRE_100KHZ)
		 *
		 * @brief	setup I2C bus.
		 *
		 * @param	u8mode	(Optional) The 8mode.
		 */
		void begin(const size_type u8speed, bool bForce = true, bool b_portalt = false) {
			_speed_and_portalt = (u8speed & WIRE_CONF::WIRE_SPEED_MASK) | (b_portalt ? WIRE_CONF::WIRE_PORT_ALT_MASK : 0);
			_init_hw(bForce);
		}

		void begin() {
			if (!_has_begun()) {
				if (_speed_and_portalt & WIRE_CONF::WIRE_SPEED_MASK) {
					begin(_speed_and_portalt & WIRE_CONF::WIRE_SPEED_MASK);
				} else {
					begin(WIRE_CONF::WIRE_100KHZ, true); // default (100K force reinit)
				}
			}
		}

		inline bool _has_begun() {
			return (_speed_and_portalt & WIRE_CONF::WIRE_PORT_INIT_MASK);
		}

		void end() {
			_speed_and_portalt &= ~WIRE_CONF::WIRE_PORT_INIT_MASK; // clear init bit
			vAHI_SiMasterDisable();
		}

		/**
		 * @brief called when about to sleep.
		 */ 
		void _on_sleep() {
			_speed_and_portalt &= ~WIRE_CONF::WIRE_PORT_INIT_MASK; // clear init bit
		}

		/**
		 * @brief called when just after waking up. (re-init the hw.)
		 */ 
		void _on_wakeup() {
			_speed_and_portalt &= ~WIRE_CONF::WIRE_PORT_INIT_MASK; // clear init bit
			
			// restart it!
			if (_speed_and_portalt) {
				_init_hw();
			}
		}
		
		void setClock(uint32_t speed) {
			// do nothing
		}

		/**
		 * @fn	uint8_t requestFrom(uint8_t u8address, size_type length, bool b_stop)
		 *
		 * @brief	Request bytes from Bus with specified length.
		 *
		 * @param	u8address	The address.
		 * @param	length   	The length to read.# set APP_TWENET_BASE
		 * @param	b_stop   	True to stop. (TODO: not supported)
		 *
		 * @returns	An uint8_t.
		 */
		size_type requestFrom(uint8_t u8address, size_type length, bool b_send_stop = true) {
			if (_mode != MODE_NONE) return 0;

			// clear que firstly.
			_que.clear();
			
			// Send address with write bit set
			vAHI_SiMasterWriteSlaveAddr(u8address, !E_AHI_SI_SLAVE_RW_SET);
			vAHI_SiMasterSetCmdReg(
				E_AHI_SI_START_BIT,
				E_AHI_SI_NO_STOP_BIT,
				E_AHI_SI_NO_SLAVE_READ,
				E_AHI_SI_SLAVE_WRITE,
				E_AHI_SI_SEND_ACK,
				E_AHI_SI_NO_IRQ_ACK);
			if (!busy_wait()) return 0;

			// Read data
			while (length > 0) {
				length--;
			
				if (length == 0 && b_send_stop) {
					// send with stop
					vAHI_SiMasterSetCmdReg(
						E_AHI_SI_NO_START_BIT,
						E_AHI_SI_STOP_BIT,
						E_AHI_SI_SLAVE_READ,
						E_AHI_SI_NO_SLAVE_WRITE,
						E_AHI_SI_SEND_NACK,
						E_AHI_SI_NO_IRQ_ACK);

				}
				else {
					vAHI_SiMasterSetCmdReg(
						E_AHI_SI_NO_START_BIT,
						E_AHI_SI_NO_STOP_BIT,
						E_AHI_SI_SLAVE_READ,
						E_AHI_SI_NO_SLAVE_WRITE,
						E_AHI_SI_SEND_ACK,
						E_AHI_SI_NO_IRQ_ACK);
				}

				// busy wait
				while (bAHI_SiMasterPollTransferInProgress());

				// put a byte into queue
				uint8_t c = u8AHI_SiMasterReadData8();
				_que.push(c);
			}

			return length;
		}

		/**
		 * @fn	int periph_twowire::read()
		 *
		 * @brief	Read buffered data (already got at requestFrom())
		 *
		 * @returns	-1: invalid, 0-255: got data.
		 */
		inline int read() {
			int ret = -1;
			if (!_que.empty()) {
				ret = _que.front();
				_que.pop();
			}

			return ret;
		}

		size_type write(const value_type val) {
			// only transmit a byte (command)
			value_type c[1] = { val };
			size_type r = write(c, 1);
			return r;
		}
		
		size_type write(const value_type* p_data, size_type quantity) {
			int ct = 0;

			if (_mode == MODE_TX) {
				if (_last_write) {
					_last_write = false;
					if (!cmd_tx_no_stop()) return 0;
				}

				while (quantity > 0) {
					vAHI_SiMasterWriteData8(*p_data++);
					
					quantity--; 
					ct++;

					if (quantity == 0) { 
						_last_write = true;
					}
					else {
						if (!cmd_tx_no_stop()) return 0;
					}
				}
			}

			return ct;
		}

		/**
		 * @fn	template <typename T, int S> inline size_type periph_twowire::write(const T(&ary)[S])
		 *
		 * @brief	write method by fixed array reference.
		 *
		 * @tparam	T	Generic type parameter.
		 * @tparam	S	Type of the s.
		 * @param	ary	The array.
		 *
		 * @returns	A size_type.
		 */
		template <typename T, int S>
		inline size_type write(const T(&ary)[S]) {
			static_assert(sizeof(T) == 1, "array type should be char type.");
			return write((const value_type*)(&ary[0]), S);
		}

		/**
		 * @fn	size_type periph_twowire::write(std::initializer_list<value_type>&& list)
		 *
		 * @brief	write to 2wire passing arguments by initializer { ... }.
		 *
		 * @param	list	The list to write.
		 *
		 * @returns	A size_type.
		 */
		size_type write(std::initializer_list<const value_type>&& list) {
			size_type ct = 0;
			for (const value_type x : list) { write(x); ct++; }
			return ct;
		}

		
		/**
		 * @fn	void periph_twowire::beginTransmission(uint8_t address)
		 *
		 * @brief	begin() write operation
		 *
		 * @param	address	The address.
		 */
		void beginTransmission(uint8_t address) {
			// set destination addr
			vAHI_SiMasterWriteSlaveAddr(address, E_AHI_SI_SLAVE_RW_SET);
			vAHI_SiMasterSetCmdReg(
				E_AHI_SI_START_BIT,
				E_AHI_SI_NO_STOP_BIT,
				E_AHI_SI_NO_SLAVE_READ,
				E_AHI_SI_SLAVE_WRITE,
				E_AHI_SI_SEND_ACK,
				E_AHI_SI_NO_IRQ_ACK);

			if (!busy_wait()) return;

			// on success, _addr is set properly
			_mode = MODE_TX;
			_addr = address;
			_last_write = false;
		}

		/**
		 * @fn	size_type periph_twowire::endTransmission(bool sendStop = true)
		 *
		 * @brief	Ends a transmission
		 *
		 * @param	sendStop	(Optional) True to send stop bit.
		 *
		 * @returns	A size_type.
		 */
		uint8_t endTransmission(bool sendStop = true) {
			// clear destination addr
			_mode = MODE_NONE;
			_addr = ADDR_UNSET;
			_last_write = false;

			if (sendStop) {
				// stop message
				if (!cmd_tx_stop()) {
					return 4;
				}
			} else {
				// no stop
				if (!cmd_tx_no_stop()) {
					return 4;
				}
			}

			return 0;
		}

		/**
		 * @fn	size_type periph_twowire::available()
		 *
		 * @brief	Gets the available buffer size.
		 *
		 * @returns	size of buffer to read.
		 */
		size_type available() {
			return _que.size();
		}

		/**
		 * @fn	void periph_twowire::beginReceive(uint8_t address)
		 *
		 * @brief	Begins a receive
		 *
		 * @param	address	The address.
		 */
		void beginReceive(uint8_t address) {
			/* Send address with write bit set */
			vAHI_SiMasterWriteSlaveAddr(address, !E_AHI_SI_SLAVE_RW_SET);

			vAHI_SiMasterSetCmdReg(
				E_AHI_SI_START_BIT,
				E_AHI_SI_NO_STOP_BIT,
				E_AHI_SI_NO_SLAVE_READ,
				E_AHI_SI_SLAVE_WRITE,
				E_AHI_SI_SEND_ACK,
				E_AHI_SI_NO_IRQ_ACK);

			if (!busy_wait()) return;

			_mode = MODE_RX;
			_addr = address;
		}

		/**
		 * @fn	uint8_t periph_twowire::endReceive(bool sendStop = true)
		 *
		 * @brief	Ends a receive
		 *
		 * @param	sendStop	(Optional) True to send stop.
		 *
		 * @returns	An uint8_t.
		 */
		uint8_t endReceive(bool sendDummyStop = true) {
			_mode = MODE_NONE;
			_addr = ADDR_UNSET;

			if (sendDummyStop) {
				vAHI_SiMasterSetCmdReg(
					E_AHI_SI_NO_START_BIT,
					E_AHI_SI_STOP_BIT,
					E_AHI_SI_SLAVE_READ,
					E_AHI_SI_NO_SLAVE_WRITE,
					E_AHI_SI_SEND_NACK,
					E_AHI_SI_NO_IRQ_ACK);

				if (!busy_wait()) return 4; // 4 is just reffering endTransmission().
				return 0;
			}
			else {
				return 0;
			}
		}

		int receive(bool sendStop = false) {
			if (_mode == MODE_RX) {
				// read a byte
				bool bwait = sendStop ? cmd_rx_stop() : cmd_rx_no_stop();
				if (!bwait) {
					return -1;
				}

				if (sendStop) MWX_DebugMsg(DLVL, "<S>");
				
				// put a byte into queue
				uint8_t c = u8AHI_SiMasterReadData8();
				return c;
			}
			else {
				return -1;
			}
		}

		reader get_reader(uint8_t address, uint8_t read_count = 0) {
			return reader(*this, address, read_count);
		}

		writer get_writer(uint8_t address) {
			return writer(*this, address);
		}

		bool probe(uint8_t address) {
			if (_mode == MODE_NONE) {
				beginTransmission(address);
				int stat = endTransmission();
				if (stat == 0) {
					return true;
				}
			}
			return false;
		}

	private:
		/**
		 * @fn	bool periph_twowire::busy_wait(void)
		 *
		 * @brief	busy wait of 2wire operation.
		 *
		 * @returns	True if it succeeds, false if it fails (disconnect, etc.).
		 */
		bool busy_wait(void) {
			// busy wait
			while (bAHI_SiMasterPollTransferInProgress());

			if (bAHI_SiMasterPollArbitrationLost() | bAHI_SiMasterCheckRxNack()) {
				vAHI_SiMasterSetCmdReg(
					E_AHI_SI_NO_START_BIT,
					E_AHI_SI_STOP_BIT,
					E_AHI_SI_NO_SLAVE_READ,
					E_AHI_SI_SLAVE_WRITE,
					E_AHI_SI_SEND_ACK,
					E_AHI_SI_NO_IRQ_ACK);
				MWX_DebugMsg(DLVL, "{ERR:busy_wait()}");
				return false;
			}
			return true;
		}

		inline bool cmd_tx_no_stop() {
			vAHI_SiMasterSetCmdReg(
				E_AHI_SI_NO_START_BIT,
				E_AHI_SI_NO_STOP_BIT,
				E_AHI_SI_NO_SLAVE_READ,
				E_AHI_SI_SLAVE_WRITE,
				E_AHI_SI_SEND_ACK,
				E_AHI_SI_NO_IRQ_ACK);

			MWX_DebugMsg(DLVL, "N");
			return busy_wait();
		}

		inline bool cmd_tx_stop() {
			vAHI_SiMasterSetCmdReg(
				E_AHI_SI_NO_START_BIT,
				E_AHI_SI_STOP_BIT,
				E_AHI_SI_NO_SLAVE_READ,
				E_AHI_SI_SLAVE_WRITE,
				E_AHI_SI_SEND_ACK,
				E_AHI_SI_NO_IRQ_ACK);

			MWX_DebugMsg(DLVL, "S");
			return busy_wait();
		}

		inline bool cmd_rx_no_stop() {
			vAHI_SiMasterSetCmdReg(
				E_AHI_SI_NO_START_BIT,
				E_AHI_SI_NO_STOP_BIT,
				E_AHI_SI_SLAVE_READ,
				E_AHI_SI_NO_SLAVE_WRITE,
				E_AHI_SI_SEND_ACK,
				E_AHI_SI_NO_IRQ_ACK);

			// busy wait
			return busy_wait();
		}

		inline bool cmd_rx_stop() {
			vAHI_SiMasterSetCmdReg(
				E_AHI_SI_NO_START_BIT,
				E_AHI_SI_STOP_BIT,
				E_AHI_SI_SLAVE_READ,
				E_AHI_SI_NO_SLAVE_WRITE,
				E_AHI_SI_SEND_NACK,
				E_AHI_SI_NO_IRQ_ACK);

			// busy wait
			return busy_wait();
		}

		void _init_hw(bool bForce = true) {
			if (_speed_and_portalt) {
				if (!(_speed_and_portalt & WIRE_CONF::WIRE_PORT_INIT_MASK) || bForce) {
					if (_speed_and_portalt & WIRE_CONF::WIRE_PORT_ALT_MASK) {
						vAHI_SiSetLocation(TRUE);
					}
					vAHI_SiMasterConfigure(TRUE, FALSE, _speed_and_portalt & WIRE_CONF::WIRE_SPEED_MASK);
					_speed_and_portalt |= WIRE_CONF::WIRE_PORT_INIT_MASK;
				}
			}
		}

	};

}}
