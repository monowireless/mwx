/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdint>
#include <utility>
#include "mwx_common.hpp"
#include "mwx_utils_crc8.hpp"

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

	// for div10(),div100(),div1000()
	struct div_result_i32 {
		int32_t quo; // quotient
		int32_t rem; // remainder
		bool b_neg;  // true if negative
	};

	// div10
	//   compute division and remains by 10 without performing / nor %.
	// return: div_result_i32
	div_result_i32 div10(int32_t val);

	// div100 (val can be up to 999999=>9999.99)
	//   compute division and remains by 100 without performing / nor %.
	// return: div_result_i32
	div_result_i32 div100(int32_t val);

	// div1000 (val can be up to 9999999>9999.999)
	//   compute division and remains by 1000 without performing / nor %.
	// return: div_result_i32
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

