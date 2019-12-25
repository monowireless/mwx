#pragma once

 /* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
  * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
  * AGREEMENT).                                                   */

#include <cstdint>
#include <iterator>
#include "Interrupt.h"

#include "mwx_debug.h"
#include "mwx_utils_alloc.hpp"


namespace mwx { inline namespace L1 {
	struct _intctl_none {
		void enter() {}
	};

	struct _intctl_jn516x {
		MICRO_INT_STORAGE;

		_intctl_jn516x() { }

		void enter() {
			MICRO_INT_ENABLE_ONLY(0);
		}

		~_intctl_jn516x() {
			MICRO_INT_RESTORE_STATE();
		}
	};

	template <typename T, class alloc, class INTCTL> class smplque;

	template <typename T, class alloc, class INTCTL = _intctl_none>
	class iter_smplque {

	public:
		typedef smplque<T, alloc, INTCTL> BODY;
	
		typedef iter_smplque self_type;
		typedef T value_type;
		typedef T& reference;
		typedef T* pointer;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;

	private:
		uint16_t _pos;
		BODY* _body;

	public:
		iter_smplque() : _pos(0), _body(nullptr) {}
		iter_smplque(uint16_t pos, BODY* body) : _pos(pos), _body(body) {
			//MWX_DebugMsg(0, "{C:%d}", pos);
		}

		inline self_type& operator ++() {
			_pos++;
			return *this;
		}

		inline self_type operator ++(int) {
			self_type p(_pos, _body);
			operator++();
			return p;
		}

		inline reference operator *() {
			return (*_body)[_pos];
		}

		inline bool operator == (self_type& itr2) {
			return !operator != (itr2);
		}
		inline bool operator == (self_type&& itr2) {
			return !operator != (itr2);
		}

		inline bool operator != (self_type& itr2) {
			return !(_pos == itr2._pos && _body == itr2._body);
		}
		inline bool operator != (self_type&& itr2) {
			return !(_pos == itr2._pos && _body == itr2._body);
		}
	};


	template <typename T, class alloc, class INTCTL=_intctl_none>
	class smplque : public INTCTL, public alloc {
		// T _buff[SIZE];
		uint16_t _head;
		uint16_t _tail;
		uint16_t _ct;

	public:
		typedef iter_smplque<T, alloc, INTCTL> iterator;
		typedef T value_type;

	public:
		smplque() : _head(0), _tail(0), _ct(0) {}
		~smplque() {}

		inline bool empty() {
			return _ct == 0;
		}

		inline uint16_t size() {
			return _ct;
		}

		inline bool is_full() {
			return _ct == alloc::super::_size;
		}

		inline uint16_t capacity() {
			return alloc::super::_size;
		}

		inline void clear() {
			INTCTL intr_ctl; // disable interrupt in this scope.
			intr_ctl.enter();

			_head = 0;
			_tail = 0;
			_ct = 0;
		}

		inline T* push_no_assign() {
			if (_ct < alloc::super::_size) {
				INTCTL intr_ctl; // disable interrupt in this scope.
				intr_ctl.enter();

				T* ptr = &alloc::super::_p[_head++];

				if (_head >= alloc::super::_size) {
					_head = 0;
				}
				_ct++;

				return ptr;
			}
			else {
				return nullptr;
			}
		}

		inline void push(T&& c) {
			if (_ct < alloc::super::_size) {
				INTCTL intr_ctl; // disable interrupt in this scope.
				intr_ctl.enter();

				alloc::super::_p[_head++] = c;
				if (_head >= alloc::super::_size) {
					_head = 0;
				}
				_ct++;
			}
		}

		inline void push(T& c) {
			if (_ct < alloc::_size) {
				INTCTL intr_ctl; // disable interrupt in this scope.
				intr_ctl.enter();

				alloc::super::_p[_head++] = c;
				if (_head >= alloc::super::_size) {
					_head = 0;
				}
				_ct++;
			}
		}

		inline void pop() {
			if (_ct > 0) {
				INTCTL intr_ctl; // disable interrupt in this scope.
				intr_ctl.enter();

				_tail++;

				if (_tail >= alloc::super::_size) {
					_tail = 0;
				}

				_ct--;
			}
		}

		inline T& front() {
			return alloc::super::_p[_tail];
		}

		inline T& back() {
			return alloc::super::_p[_head];
		}

		inline iterator begin() {
			return iterator(0, this);
		}

		inline iterator end() {
			return iterator(_ct, this);
		}

		inline T& operator[] (int i) {
			int idx = _tail + i;
			if (idx >= alloc::super::_size) {
				idx -= alloc::super::_size;
			}
			return alloc::super::_p[idx];
		}

		inline T& pop_front() {
			int idx = _tail;
			pop(); // this does not destroy object at tail.
			return alloc::super::_p[idx];
		}
	};
	
	// rename
	template <typename T, int N, class Intr=_intctl_none>
	using smplque_local =  mwx::smplque<T, mwx::alloc_local<T, N>, Intr>;
	template <typename T, class Intr=_intctl_none>
	using smplque_attach =  mwx::smplque<T, mwx::alloc_attach<T>, Intr>;
	template <typename T, class Intr=_intctl_none>
	using smplque_heap =  mwx::smplque<T, mwx::alloc_heap<T>, Intr>;
}}
