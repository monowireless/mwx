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
	inline uint8_t G_OCTET(uint8_t*& p) {
		return *(p)++; 
	}
	inline uint16_t G_WORD(uint8_t*& p) {
		uint32_t r = *p++;
		r = (r << 8) + *p++;
		return r;
	}
	inline uint32_t G_DWORD(uint8_t*& p) {
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

}} // TWEUTILS

