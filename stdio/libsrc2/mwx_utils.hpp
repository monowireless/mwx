/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdint>
#include <utility>
#include "mwx_common.hpp"
#include "mwx_utils_crc8.hpp"
//#include "tweprintf.h"

namespace mwx { inline namespace L1 {
	// read uint8_t, uint16_t, uint32_t as bigendian from specified pointer `p' and increment pointer.
	//   e.g.)
	//     uint8 buf[128] = { 0x01, 0x12, 0x34, 0xab, 0xcd}, *p = buf;
	//     uint8_t b; uint32_t d;
	//     b = G_OCTET(p); // got 0x01, now p = buf + 1
	//     d = G_DWORD(p); // got 0x1234abcd, now p = buf + 5
	inline uint8_t G_OCTET(const uint8_t*& p) {
		return *(p)++; 
	}
	inline uint16_t G_WORD(const uint8_t*& p) {
		uint32_t r = *p++;
		r = (r << 8) + *p++;
		return r;
	}
	inline uint32_t G_DWORD(const uint8_t*& p) {
		uint32_t r = *p++;
		r = (r << 8) + *p++;
		r = (r << 8) + *p++;
		r = (r << 8) + *p++;
		return r;
	}

	// store uint8_t, uint16_t, uint32_t as bigendian to specified pointer `q' and increment them.
	//   e.g.)
	//     uint8 buf[128], *q = buf;
	//     S_OCTET(q, 0x01); // store { 0x01 }, now q = buf+1
	//     S_DWORD(q, 0x1234abcd) // store {  0x12, 0x34, 0xab, 0xcd }, now q = buf+5
	//     --> { 0x01, 0x12, 0x34, 0xab, 0xcd }
	inline uint8_t& S_OCTET(uint8_t*& q, uint8_t c) {
		*q++ = c;
		return *q;
	}
	inline uint8_t& S_WORD(uint8_t*& q, uint16_t c) {
		*(q) = ((c) >> 8) & 0xff; (q)++;
		*(q) = ((c) & 0xff); (q)++;
		return *q;
	}
	inline uint8_t& S_DWORD(uint8_t*& q, uint32_t c) {
		*(q) = ((c) >> 24) & 0xff; (q)++;
		*(q) = ((c) >> 16) & 0xff; (q)++;
		*(q) = ((c) >>  8) & 0xff; (q)++;
		*(q) = ((c) & 0xff); (q)++;
		return *q;
	}

	// encode/decode volt 2000-3600mv into 8bit value
	inline uint8_t EncodeVolt(uint16_t m) {
		return (m < 1950 ? 0 : (m > 3650 ? 255 : (m <= 2802 ? ((m-1950+2)/5) : ((m-2800-5)/10+171)) ));
	}

	// encode/decode volt 2000-3600mv into 8bit value
	inline uint16_t DecodeVolt(uint8_t i) {
		return (i <= 170 ? (1950+i*5) : (2800+(i-170)*10) );
	}

	// packing bits with given arguments, which specifies bit position.
	//   pack_bits(5, 0, 1) -> (b100011) bit0,1,5 are set.
	template <typename Head>
	constexpr uint32_t pack_bits(Head head) { return  1UL << head; }
	template <typename Head, typename... Tail>
	constexpr uint32_t pack_bits(Head head, Tail&&... tail) {
		return (1UL << head) | pack_bits(std::forward<Tail>(tail)...);
	}

	// expand argumetns bits from `bm' and pack them into single bitmap.
	//   uint32_t b1 = 0x12; // (b00010010)
	//   uint32_t b2 = collect_bits(b1, 4, 2, 1, 0); // (b1010) bit4==1, bit2==0, bit1==1, bit0==0
	template <typename Head>
	constexpr uint32_t collect_bits(uint32_t bm, Head head) { return ((bm & (1UL << head)) ? 1 : 0); }
	template <typename Head, typename... Tail>
	constexpr uint32_t collect_bits(uint32_t bm, Head head, Tail&&... tail) {
		return (((bm & (1UL << head)) ? 1 : 0) << sizeof...(tail)) | collect_bits(bm, std::forward<Tail>(tail)...);
	}

	/**
	 * @brief parameter defs for for div_result_i32::format()
	 */
	struct DIVFMT {
		static const int STD = 0; // displays with minimul digits (no padding, no positive sign)
		static const int PAD_ZERO = 1; // set padding character as '0' instead of ' '.
		static const int SIGN_PLUS = 2; // put '+' sign if value is positive or 0.
		static const int PAD_ZERO_SIGN_PLUS = 3; // PAD_ZERO & SIGN_PLUS
		static const int SIGN_SPACE = 4; // put ' ' sign if value is positive or 0.
		static const int PAD_ZERO_SIGN_SPACE = 5; // PAD_ZERO & SIGN_SPACE
	};

	/**
	 * @brief helper class for div_result_i32::format()
	 *        - keep char[] buffer for output
	 *        - interface to report text pointer.
	 */
	class _div_chars {
		static const int MAXBUFF=15;
		char _buff[MAXBUFF + 1];
		
		/**
		 * @brief index counter.
		 */
		char& _idx() {
			return _buff[MAXBUFF];
		}

		/**
		 * @brief index counter (const reference)
		 */
		char _idx() const {
			return _buff[MAXBUFF];
		}

	public:
		/**
		 * @brief Construct a new div chars object
		 */
		_div_chars() {
			// set buffer ends as NUL char.
			_buff[MAXBUFF-1] = 0;

			// set index counter pointing NUL char.
			_idx() = MAXBUFF-1;
		}

		/**
		 * @brief push one char at the head of buffer.
		 * 		  note: if `_idx()' reaches buffer head, ignore request.
		 * 
		 * @param c 
		 */
		void push_front(char c) {
			if (_idx() > 0) { --_idx(); _buff[(uint8_t)_idx()] = c; }
		}

		/**
		 * @brief begin() iterator.
		 * 
		 * @return const char* 
		 */
		const char* begin() const { return &_buff[(uint8_t)_idx()]; }
		
		/**
		 * @brief end() iterator.
		 * 
		 * @return const char* 
		 */
		const char* end() const { return &_buff[MAXBUFF-1]; }

		/**
		 * @brief exporting text buffer pointer as (const char*)
		 * 
		 * @return const char* 
		 */
		const char* c_str() const { return begin(); }
		
		/**
		 * @brief exporting text buffer pointer as (const char*)
		 * 
		 * @return const char* 
		 */
		operator const char*() const { return begin(); }
	};

	/**
	 * @brief store result of divide functions (div10(),div100(),div1000())
	 */
	struct div_result_i32 {
	private:
		static const int ZERO_PAD = 0x01; 
		static const int SIGN_PLUS = 0x02;
		static const int SIGN_SPACE = 0x04;

	public:
		int32_t quo; // quotient
		int16_t rem; // remainder
		uint8_t b_neg;  // true if negative
		uint8_t digits_rem; // digits of remainder

	public:
		/**
		 * @brief format for printing
		 * 
		 * @param dig_quo if set > 0, keep at least dig_quo digits (except +/- sign)
		 * @param opt  see DIVFMT.
		 * @return _div_chars returns as helper class which keeps converted string.
		 */
		_div_chars format(int dig_quo = 0, uint32_t opt = DIVFMT::STD) const;
	};

	/**
	 * @brief compute division and remains by 10 without performing / nor %.
	 * 
	 * @param val the value to divide.
	 * @return div_result_i32 the result of division (quotient/remainder/+,-sign)
	 */
	div_result_i32 div10(int32_t val);

	/**
	 * @brief compute division and remains by 100 without performing / nor %.
	 *
	 * @param val the value top divide (note: possible value can be -999999 to 999999).
	 * @return div_result_i32 the result of division (quotient/remainder/+,-sign)
	 */
	div_result_i32 div100(int32_t val);

	/**
	 * @brief compute division and remains by 1000 without performing / nor %.
	 * 
	 * @param val the value top divide (note: possible value can be -9999999 to 9999999).
	 * @return div_result_i32 the result of division (quotient/remainder/+,-sign)
	 */
	div_result_i32 div1000(int32_t val);

	/**
	 * @brief scale 0..1000 to 0..127
	 * 
	 * @param x original value 0..1000, if computed value > 127, returns 127
	 * @return uint8_t scaled value 0..127
	 */
	static inline uint8_t scale_1000_to_127u8(uint16_t x) { return x > 1000 ? 127 : (16646*x+65000) >> 17; } // approx. (127*x/1000)

	/**
	 * @brief scale 0..127 to 0.1000
	 * 
	 * @param x original value (0..127)
	 * @return uint16_t scaled value 0..1000
	 */
	static inline uint16_t scale_127u8_to_1000(uint8_t x) {
		uint16_t y = (2064000UL*uint32_t(x)+131072)>>18; return y > 1000 ? 1000 : y; }

	/**
	 * @brief scale 0..1000 to 0..255
	 * 
	 * @param x original value 0..1000, if computed value > 127, returns 255
	 * @return uint8_t scaled value 0..255
	 */
	static inline uint8_t scale_1000_to_255u8(uint16_t x) { return x > 1000 ? 255 : (33423*x+65000) >> 17; } // approx. (255*x/1000)

	/**
	 * @brief scale 0..255 to 0..1000
	 * 
	 * @param x original value (0..255)
	 * @return uint16_t scaled value (0..1000)
	 */
	static inline uint16_t scale_255u8_to_1000(uint8_t x) {
		uint16_t y = (1028000UL*uint32_t(x)+131072)>>18; return y > 1000 ? 1000 : y; }

	/**
	 * @brief scale 0..1000 to 0..256
	 *        note: 256 cannot be returned, but 255.
	 * 
	 * @param x original value 0..1000, if computed value > 255, returns 255
	 * @return uint8_t scaled value 0..255
	 */
	static inline uint8_t scale_1000_to_256u8(uint16_t x) { return x > 995 ? 255 : (33554*x+66000) >> 17; } // approx. (256*x/1000)
	
	/**
	 * @brief scale 0..256 to 0..1000
	 * 
	 * @param x original value (0..256)
	 * @return uint16_t scaled value 0..1000
	 */
	static inline uint16_t scale_256u16_to_1000(uint16_t x) {
		uint16_t y = (1024000UL*uint32_t(x)+131072)>>18; return y > 1000 ? 1000 : y; }

}} // TWEUTILS

