/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdint>
#include <cstring>
#include <utility>
#include <type_traits>

#include "twecommon.h"
#include "mwx_utils_smplbuf.hpp"

namespace mwx { inline namespace L1 {
	static inline int _expand_bytes(const uint8_t* b, const uint8_t* e) {
		return 0;
	}

	template <typename... Tail>
	int _expand_bytes(const uint8_t* b, const uint8_t* e, uint32_t& head, Tail&&... tail);

	template <typename... Tail>
	int _expand_bytes(const uint8_t* b, const uint8_t* e, uint16_t& head, Tail&&... tail);

	template <typename... Tail>
	int _expand_bytes(const uint8_t* b, const uint8_t* e, uint8_t& head, Tail&&... tail);

	template <typename... Tail, int S>
	int _expand_bytes(const uint8_t* b, const uint8_t* e, uint8_t(&head)[S], Tail&&... tail);

	template <typename... Tail>
	int _expand_bytes(const uint8_t* b, const uint8_t* e, uint32_t& head, Tail&&... tail)
	{
		if (b + 4 <= e) {
			head = (*(b) << 24) | (*(b+1) << 16) | (*(b+2) << 8) | (*(b+3) << 0);
			return 4 + _expand_bytes(b + 4, e, std::forward<Tail>(tail)...);
		}
		else {
			return 0;
		}
	}

	template <typename... Tail>
	int _expand_bytes(const uint8_t* b, const uint8_t* e, uint16_t& head, Tail&&... tail)
	{
		if (b + 2 <= e) {
			head = (*(b) << 8) | (*(b+1) << 0);
			return 2 + _expand_bytes(b + 2, e, std::forward<Tail>(tail)...);
		}
		else {
			return 0;
		}
	}

	template <typename... Tail>
	int _expand_bytes(const uint8_t* b, const uint8_t* e, uint8_t& head, Tail&&... tail)
	{
		if (b + 1 <= e) {
			head = (*(b) << 0);
			return 1 + _expand_bytes(b + 1, e, std::forward<Tail>(tail)...);
		}
		else {
			return 0;
		}
	}

	template <typename T, typename I, typename... Tail>
	int _expand_bytes(const uint8_t* b, const uint8_t* e, std::pair<T*, I> head, Tail&&... tail) {
		static_assert(sizeof(T) == 1, "should be char/uint8_t type.");
		static_assert(std::is_integral<T>::value, "2nd arg should be integral.");
		T* p = std::get<0>(head);
		int len = std::get<1>(head);

		if (b + len < e) {
			for (int i = 0; i < len; i++) {
				*p++ = *b++;
			}
			return len + _expand_bytes(b, e, std::forward<Tail>(tail)...);
		}
		else {
			return 0;
		}
	}


	template <typename... Tail, int S>
	int _expand_bytes(const uint8_t* b, const uint8_t* e, uint8_t(&head)[S], Tail&&... tail) {
		if (b + S <= e) {
			memcpy(head, b, S);
			return S + _expand_bytes(b + S, e, std::forward<Tail>(tail)...);
		}
		else {
			return 0;
		}
	}

	template <typename Head, typename... Tail>
	const uint8_t* expand_bytes(const uint8_t* b, const uint8_t* e, Head&& head, Tail&&... tail) {
		int ret = _expand_bytes(b, e, std::forward<Head>(head), std::forward<Tail>(tail)...);
		if (ret) {
			return b + ret;
		}
		else {
			return nullptr;
		}
	}
}}

namespace mwx { inline namespace L1 {
	static inline int _pack_bytes(uint8_t* b, uint8_t* e) {
		return 0;
	}

	template <typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, const uint32_t& head, Tail&&... tail);

	template <typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, const uint16_t& head, Tail&&... tail);

	template <typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, const uint8_t& head, Tail&&... tail);

	template <int S, typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, const uint8_t(&head)[S], Tail&&... tail);
	
	template <typename T, typename I, typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, std::pair<T*,I> head, Tail&&... tail);

	template <typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, const uint32_t& head, Tail&&... tail)
	{
		if (b + 4 <= e) {
			*b++ = static_cast<uint8_t>((head >> 24) & 0xff);
			*b++ = static_cast<uint8_t>((head >> 16) & 0xff);
			*b++ = static_cast<uint8_t>((head >> 8) & 0xff);
			*b++ = static_cast<uint8_t>((head >> 0) & 0xff);
			return 4 + _pack_bytes(b, e, std::forward<Tail>(tail)...);
		}
		else {
			return 0;
		}
	}

	template <typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, const uint16_t& head, Tail&&... tail)
	{
		if (b + 2 <= e) {
			*b++ = static_cast<uint8_t>((head >> 8) & 0xff);
			*b++ = static_cast<uint8_t>((head >> 0) & 0xff);
			return 2 + _pack_bytes(b, e, std::forward<Tail>(tail)...);
		}
		else {
			return 0;
		}
	}

	template <typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, const uint8_t& head, Tail&&... tail)
	{
		if (b + 1 <= e) {
			*b++ = static_cast<uint8_t>((head >> 0) & 0xff);
			return 1 + _pack_bytes(b, e, std::forward<Tail>(tail)...);
		}
		else {
			return 0;
		}
	}

	// S bytes of array
	template <int S, typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, const uint8_t(&head)[S], Tail&&... tail) {
		if (b + S <= e) {
			memcpy(b, head, S);
			return S + _pack_bytes(b + S, e, std::forward<Tail>(tail)...);
		}
		else {
			return 0;
		}
	}
	
	template <typename T, typename S, typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, std::pair<T*,S*> head, Tail&&... tail) {
		static_assert(sizeof(T) == 1, "should be char/uint8_t type.");
		static_assert(sizeof(S) == 1, "1st arg should be char/uint8_t type.");
		T* p = std::get<0>(head);
		int len = std::get<1>(head) - p;
		
		if (b + len < e) {
			for (int i = 0; i < len; i++) {
				*b++ = *p++;
			}
			return len + _pack_bytes(b, e, std::forward<Tail>(tail)...);
		} else {
			return 0;
		}
	}

	template <typename T, typename I, typename... Tail>
	int _pack_bytes(uint8_t* b, uint8_t* e, std::pair<T*,int> head, Tail&&... tail) {
		static_assert(sizeof(T) == 1, "should be char/uint8_t type.");
		T* p = std::get<0>(head);
		int len = std::get<1>(head);

		if (b + len < e) {
			for (int i = 0; i < len; i++) {
				*b++ = *p++;
			}
			return len + _pack_bytes(b, e, std::forward<Tail>(tail)...);
		} else {
			return 0;
		}
	}

	// packing bytes into an bytes array with begin()/end() iterator
	template <typename Head, typename... Tail>
	uint8_t* pack_bytes(uint8_t* b, uint8_t* e, Head&& head, Tail&&... tail) {
		int ret = _pack_bytes(b, e, std::forward<Head>(head), std::forward<Tail>(tail)...);
		if (ret) {
			return b + ret;
		}
		else {
			return nullptr;
		}
	}
}}

namespace mwx { inline namespace L1 {
	template <typename Cnt>
	static inline int _pack_bytes_pb(Cnt& c) {
		return 0;
	}
	
	template <typename Cnt, int N,  class alloc, typename... Tail>
	int _pack_bytes_pb(Cnt& c, mwx::smplbuf<uint8_t, mwx::alloc_local<uint8_t, N>>& head, Tail&&... tail);

	template <typename Cnt, class alloc, typename... Tail>
	int _pack_bytes_pb(Cnt &c, mwx::smplbuf<uint8_t, alloc>& head, Tail&&... tail);

	template <typename Cnt, typename... Tail>
	int _pack_bytes_pb(Cnt& c, const uint32_t& head, Tail&&... tail);

	template <typename Cnt, typename... Tail>
	int _pack_bytes_pb(Cnt& c, const uint16_t& head, Tail&&... tail);

	template <typename Cnt, typename... Tail>
	int _pack_bytes_pb(Cnt& c, const uint8_t& head, Tail&&... tail);

	template <typename Cnt, int S, typename... Tail>
	int _pack_bytes_pb(Cnt& c, const uint8_t(&head)[S], Tail&&... tail);

	template <typename Cnt, typename T, typename I, typename... Tail>
	int _pack_bytes_pb(Cnt &c, std::pair<T*, I> head, Tail&&... tail);

	template <typename Cnt, typename... Tail>
	int _pack_bytes_pb(Cnt& c, const uint32_t& head, Tail&&... tail)
	{
		c.push_back(static_cast<uint8_t>((head >> 24) & 0xff));
		c.push_back(static_cast<uint8_t>((head >> 16) & 0xff));
		c.push_back(static_cast<uint8_t>((head >> 8) & 0xff));
		c.push_back(static_cast<uint8_t>((head >> 0) & 0xff));
		return 4 + _pack_bytes_pb(c, std::forward<Tail>(tail)...);
	}

	template <typename Cnt, typename... Tail>
	int _pack_bytes_pb(Cnt& c, const uint16_t& head, Tail&&... tail)
	{
		c.push_back(static_cast<uint8_t>((head >> 8) & 0xff));
		c.push_back(static_cast<uint8_t>((head >> 0) & 0xff));
		return 2 + _pack_bytes_pb(c, std::forward<Tail>(tail)...);
	}

	template <typename Cnt, typename... Tail>
	int _pack_bytes_pb(Cnt& c, const uint8_t& head, Tail&&... tail)
	{
		c.push_back(static_cast<uint8_t>((head >> 0) & 0xff));
		return 1 + _pack_bytes_pb(c, std::forward<Tail>(tail)...);
	}

	// copy S bytes of uint8_t[]
	template <typename Cnt, int S, typename... Tail>
	int _pack_bytes_pb(Cnt &c, const uint8_t(&head)[S], Tail&&... tail) {
		uint8_t* p = (uint8_t*)(&head[0]);
		for (int i = 0; i < S; i++) {
			c.push_back(*p++);
		}
		return S + _pack_bytes_pb(c, std::forward<Tail>(tail)...);
	}

	template <typename Cnt, typename T, typename S, typename... Tail>
	int _pack_bytes_pb(Cnt &c, std::pair<T*, S*> head, Tail&&... tail) {
		static_assert(sizeof(T) == 1, "1st arg should be char/uint8_t type.");
		static_assert(sizeof(S) == 1, "1st arg should be char/uint8_t type.");
		T* p = std::get<0>(head);
		int len = std::get<1>(head) - p;
		for (int i = 0; i < len; i++) {
			c.push_back(*p++);
		}
		return len + _pack_bytes_pb(c, std::forward<Tail>(tail)...);
	}

	template <typename Cnt, typename T, typename... Tail>
	int _pack_bytes_pb(Cnt &c, std::pair<T*, int> head, Tail&&... tail) {
		static_assert(sizeof(T) == 1, "1st arg should be char/uint8_t type.");
		T* p = std::get<0>(head);
		int len = std::get<1>(head);
		for (int i = 0; i < len; i++) {
			c.push_back(*p++);
		}
		return len + _pack_bytes_pb(c, std::forward<Tail>(tail)...);
	}

	// limited to smplbuf<uint8_t, N>
	template <typename Cnt, int N, typename... Tail>
	int _pack_bytes_pb(Cnt &c, mwx::smplbuf<uint8_t, mwx::alloc_local<uint8_t, N>>& head, Tail&&... tail) {
		for (auto&& x : head) {
			c.push_back(x);
		}
		return head.size() + _pack_bytes_pb(c, std::forward<Tail>(tail)...);
	}

	template <typename Cnt, class alloc, typename... Tail>
	int _pack_bytes_pb(Cnt &c, mwx::smplbuf<uint8_t, alloc>& head, Tail&&... tail) {
		for (auto&& x : head) {
			c.push_back(x);
		}
		return head.size() + _pack_bytes_pb(c, std::forward<Tail>(tail)...);
	}

	// packing bytes into an bytes array with puch_back() operation
	template <typename Cnt, typename Head, typename... Tail>
	Cnt& pack_bytes(Cnt &c, Head&& head, Tail&&... tail) {
		_pack_bytes_pb(c, std::forward<Head>(head), std::forward<Tail>(tail)...);
		return c;
	}
}}
