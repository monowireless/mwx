/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "mwx_common.hpp"

namespace mwx { inline namespace L1 {
	// Combine two Enums into one.
    template <typename A, typename B>
    class enum_combiner {
        int _i;
        // operator int() { return _i; }
    public:
        // enum_combiner(int init = 0) : _i(init) {}
        enum_combiner(A e) { _i = (int)e; }
        enum_combiner(B e) { _i = (int)e; }
        operator A() { return (A)_i; }
        operator B() { return (B)_i; }
        int to_int() { return _i; }
        bool operator ==(enum_combiner& e) { return _i == e.to_int(); }
    };
    
	// enum wapper class
	template<class _Enum, typename T = uint8_t>
	class enum_wapper
	{
	private:
		_Enum _e;
		// operator int() = delete;
	public:
		enum_wapper(const _Enum &e) { _e = e; }
		inline operator _Enum() const { return _e; }
		inline _Enum to_enum() const { return _e; }
		inline void operator =(const _Enum& x) { _e = x; }
		inline bool operator ==(const _Enum& x) const { return _e == x; }		
	};

}} // TWEUTILS

