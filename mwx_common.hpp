/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

/* NOTE: This file is include very early of includes, so those definitions here
 * should be independent from following libraries.
 */

// architecture check
#if defined(JENNIC_CHIP)
# define __TWELITE__
#elif defined(__APPLE__) || defined(__linux) || defined(_MSC_VER) || defined(__MINGW32__)
//# warning "..build on Windows/macOS/Linux. TWE_STDINOUT_ONLY defined."
# define TWE_STDINOUT_ONLY
#else
# error "Architecture need to be identified."
#endif

// type definition
#include <cstdint> // for type name
typedef char char_t;
typedef uint8_t byte;
typedef uint8_t boolean;

#ifndef NULL
#define NULL nullptr
#endif

#include <utility>

// generic API returns
namespace mwx { inline namespace L1 {
	/**
	 * @brief API returns values with success/fail and 31bits data.
	 */
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

	/**
	 * @brief placement new syntax helper.
	 *
	 *  Typical placement new description is like below:
	 *    sometype obj;
	 *    new (&obj) sometype(arg1, arg2);
	 *   
	 *  This template funcion makes it as below.
	 *    sometype obj;
	 *    pnew(pbj, arg1, arg2); 
	 * 
	 */
	template <class T, class... Args>
	T* pnew(T& obj, Args&&... args) {
		return (T*)new ((void*)&obj) T(std::forward<Args&&>(args)...);
	}
}}
