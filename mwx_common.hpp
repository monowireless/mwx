/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

// type definition
#include <cstdint> // for type name
typedef char char_t;
typedef uint8_t byte;
typedef uint8_t boolean;

#ifndef NULL
#define NULL nullptr
#endif

// generic API returns
namespace mwx { inline namespace L1 {
	class MWX_APIRET {
		uint32_t _code;
	public:
		MWX_APIRET() : _code(0) {}
		MWX_APIRET(bool b) {
			_code = (b ? 0x80000000 : 0);
		}
		MWX_APIRET(bool b, uint32_t val) {
			_code = (b ? 0x80000000 : 0) + (val & 0x7fffffff);
		}
		MWX_APIRET(uint32_t val) {
			_code = val;
		}
		inline bool is_success() const { return ((_code & 0x80000000) != 0); }
		inline bool is_fail() const { return ((_code & 0x80000000) == 0); }
		inline uint32_t get_value() const { return _code & 0x7fffffff; }
		inline operator uint32_t() const { return get_value(); }
		inline operator bool() const { return is_success(); }
	};
}}
