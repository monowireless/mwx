/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once
#pragma GCC diagnostic ignored "-Wreorder"

#include <cstdarg>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <tuple>
#include <initializer_list>

#include "twecommon.h"

#include "mwx_common.hpp"
#include "mwx_utils.hpp"

#if !defined(TWE_STDINOUT_ONLY)
#include "tweprintf.h"
#include "mwx_periph.hpp"
#include "mwx_debug.h"
#else
#include "_mwx_stdio.h"
#include "printf.h"
#endif

// for print(xx, YY);
#define DEC 10
#define HEX 16
#define OCT 8
#ifdef BIN
#undef BIN
#endif
#define BIN 2

namespace mwx { inline namespace L1 {
	static const int MWX_SEEK_END = -1;
	static const int MWX_SEEK_CUR = 1;
	static const int MWX_SEEK_SET = 0;

    typedef void (*tfcOutput)(char character, void* arg);

	class _printobj {
	public:
		typedef void (*tf_do_print)(void *myobj, tfcOutput fn, void *pvContext);

	protected:
		const char *_fmt;

		tf_do_print _pfdoprt;
		void *_pobj;

	public:
		_printobj(const char* fmt, tf_do_print pfdoprt = nullptr, void* pobj = nullptr)
				: _fmt(fmt)
				, _pobj(pobj)
				, _pfdoprt(pfdoprt) {}

        void do_print(tfcOutput fn, void *pvContext) {
			if (_pfdoprt && _pobj) {
				_pfdoprt(_pobj, fn, pvContext);
			} else {
				fctprintf(fn, pvContext, _fmt);
			}
		}
	};

	template <typename T1>
	class _printobj_1 : public _printobj {
		T1 _a1;
	public:
		_printobj_1(const char *fmt, T1 a1) 
				: _printobj(fmt, _printobj_1::_do_print, reinterpret_cast<void*>(this)) 
				, _a1(a1) {}
		static void _do_print(void *myobj, tfcOutput fn, void *pvContext) {
			if (myobj != nullptr) {
				_printobj_1 *pobj = reinterpret_cast<_printobj_1*>(myobj);
				fctprintf(fn, pvContext, pobj->_fmt, pobj->_a1);
			}
		}
	};

	template <typename T1, typename T2>
	class _printobj_2 : public _printobj {
		T1 _a1;
		T2 _a2;
	public:
		_printobj_2(const char *fmt, T1 a1, T2 a2) 
				: _printobj(fmt, _printobj_2::_do_print, reinterpret_cast<void*>(this)) 
				, _a1(a1), _a2(a2) {}
		static void _do_print(void *myobj, tfcOutput fn, void *pvContext) {
			if (myobj != nullptr) {
				_printobj_2 *pobj = reinterpret_cast<_printobj_2*>(myobj);
				fctprintf(fn, pvContext, pobj->_fmt, pobj->_a1, pobj->_a2);
			}
		}
	};

	template <typename T1, typename T2, typename T3>
	class _printobj_3 : public _printobj {
		T1 _a1;
		T2 _a2;
		T3 _a3;
	public:
		_printobj_3(const char *fmt, T1 a1, T2 a2, T3 a3) 
				: _printobj(fmt, _printobj_3::_do_print, reinterpret_cast<void*>(this)) 
				, _a1(a1), _a2(a2), _a3(a3) {}
		static void _do_print(void *myobj, tfcOutput fn, void *pvContext) {
			if (myobj != nullptr) {
				_printobj_3 *pobj = reinterpret_cast<_printobj_3*>(myobj);
				fctprintf(fn, pvContext, pobj->_fmt, pobj->_a1, pobj->_a2, pobj->_a3);
			}
		}
	};

	template <typename T1, typename T2, typename T3, typename T4>
	class _printobj_4 : public _printobj {
		T1 _a1;
		T2 _a2;
		T3 _a3;
		T4 _a4;
	public:
		_printobj_4(const char *fmt, T1 a1, T2 a2, T3 a3, T4 a4) 
				: _printobj(fmt, _printobj_4::_do_print, reinterpret_cast<void*>(this)) 
				, _a1(a1), _a2(a2), _a3(a3), _a4(a4) {}
		static void _do_print(void *myobj, tfcOutput fn, void *pvContext) {
			if (myobj != nullptr) {
				_printobj_4 *pobj = reinterpret_cast<_printobj_4*>(myobj);
				fctprintf(fn, pvContext, pobj->_fmt, pobj->_a1, pobj->_a2, pobj->_a3, pobj->_a4);
			}
		}
	};

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	class _printobj_5 : public _printobj {
		T1 _a1;
		T2 _a2;
		T3 _a3;
		T4 _a4;
		T5 _a5;
	public:
		_printobj_5(const char *fmt, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) 
				: _printobj(fmt, _printobj_5::_do_print, reinterpret_cast<void*>(this)) 
				, _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5) {}
		static void _do_print(void *myobj, tfcOutput fn, void *pvContext) {
			if (myobj != nullptr) {
				_printobj_5 *pobj = reinterpret_cast<_printobj_5*>(myobj);
				fctprintf(fn, pvContext, pobj->_fmt, pobj->_a1, pobj->_a2, pobj->_a3, pobj->_a4, pobj->_a5);
			}
		}
	};

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	class _printobj_6 : public _printobj {
		T1 _a1;
		T2 _a2;
		T3 _a3;
		T4 _a4;
		T5 _a5;
		T6 _a6;
	public:
		_printobj_6(const char *fmt, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6) 
				: _printobj(fmt, _printobj_6::_do_print, reinterpret_cast<void*>(this)) 
				, _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6) {}
		static void _do_print(void *myobj, tfcOutput fn, void *pvContext) {
			if (myobj != nullptr) {
				_printobj_6 *pobj = reinterpret_cast<_printobj_6*>(myobj);
				fctprintf(fn, pvContext, pobj->_fmt, pobj->_a1, pobj->_a2, pobj->_a3, pobj->_a4, pobj->_a5, pobj->_a6);
			}
		}
	};

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	class _printobj_7 : public _printobj {
		T1 _a1;
		T2 _a2;
		T3 _a3;
		T4 _a4;
		T5 _a5;
		T6 _a6;
		T7 _a7;
	public:
		_printobj_7(const char *fmt, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7) 
				: _printobj(fmt, _printobj_7::_do_print, reinterpret_cast<void*>(this)) 
				, _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6), _a7(a7) {}
		static void _do_print(void *myobj, tfcOutput fn, void *pvContext) {
			if (myobj != nullptr) {
				_printobj_7 *pobj = reinterpret_cast<_printobj_7*>(myobj);
				fctprintf(fn, pvContext, pobj->_fmt, pobj->_a1, pobj->_a2, pobj->_a3, pobj->_a4, pobj->_a5, pobj->_a6, pobj->_a7);
			}
		}
	};
	
	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	class _printobj_8 : public _printobj {
		T1 _a1;
		T2 _a2;
		T3 _a3;
		T4 _a4;
		T5 _a5;
		T6 _a6;
		T7 _a7;
		T8 _a8;
	public:
		_printobj_8(const char *fmt, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8) 
				: _printobj(fmt, _printobj_8::_do_print, reinterpret_cast<void*>(this)) 
				, _a1(a1), _a2(a2), _a3(a3), _a4(a4), _a5(a5), _a6(a6), _a7(a7), _a8(a8) {}
		static void _do_print(void *myobj, tfcOutput fn, void *pvContext) {
			if (myobj != nullptr) {
				_printobj_8 *pobj = reinterpret_cast<_printobj_8*>(myobj);
				fctprintf(fn, pvContext, pobj->_fmt, pobj->_a1, pobj->_a2, pobj->_a3, pobj->_a4, pobj->_a5, pobj->_a6, pobj->_a7, pobj->_a8);
			}
		}
	};

	const size_t MAX_SIZE_PRINTOBJ = sizeof(_printobj_4<double, double, double, double>);
	class mwx_format {
		uint8_t _pobj[MAX_SIZE_PRINTOBJ];

	public:
		mwx_format(const char* fmt) {
			(void)new ((void*)_pobj) _printobj(fmt);
		}
		template <typename T1>
		mwx_format(const char* fmt, T1 a1) {
			static_assert(sizeof(_printobj_1<T1>) <= MAX_SIZE_PRINTOBJ, "Pre-alloc size overflow. Check MAX_SIZE_PRINTOBJ.");
			(void)new ((void*)_pobj) _printobj_1<T1>(fmt, a1);
		}
		template <typename T1, typename T2>
		mwx_format(const char* fmt, T1 a1, T2 a2) {
			static_assert(sizeof(_printobj_2<T1, T2>) <= MAX_SIZE_PRINTOBJ, "Pre-alloc size overflow. Check MAX_SIZE_PRINTOBJ.");
			(void)new ((void*)_pobj) _printobj_2<T1, T2>(fmt, a1, a2);
		}
		template <typename T1, typename T2, typename T3>
		mwx_format(const char* fmt, T1 a1, T2 a2, T3 a3) {
			static_assert(sizeof(_printobj_3<T1, T2, T3>) <= MAX_SIZE_PRINTOBJ, "Pre-alloc size overflow. Check MAX_SIZE_PRINTOBJ.");
			(void)new ((void*)_pobj) _printobj_3<T1, T2, T3>(fmt, a1, a2, a3);
		}
		template <typename T1, typename T2, typename T3, typename T4>
		mwx_format(const char* fmt, T1 a1, T2 a2, T3 a3, T4 a4) {
			static_assert(sizeof(_printobj_4<T1, T2, T3, T4>) <= MAX_SIZE_PRINTOBJ, "Pre-alloc size overflow. Check MAX_SIZE_PRINTOBJ.");
			(void)new ((void*)_pobj) _printobj_4<T1, T2, T3, T4>(fmt, a1, a2, a3, a4);
		}
		template <typename T1, typename T2, typename T3, typename T4, typename T5>
		mwx_format(const char* fmt, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) {
			static_assert(sizeof(_printobj_5<T1, T2, T3, T4, T5>) <= MAX_SIZE_PRINTOBJ, "Pre-alloc size overflow. Check MAX_SIZE_PRINTOBJ.");
			(void)new ((void*)_pobj) _printobj_5<T1, T2, T3, T4, T5>(fmt, a1, a2, a3, a4, a5);
		}
		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		mwx_format(const char* fmt, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6) {
			static_assert(sizeof(_printobj_6<T1, T2, T3, T4, T5, T6>) <= MAX_SIZE_PRINTOBJ, "Pre-alloc size overflow. Check MAX_SIZE_PRINTOBJ.");
			(void)new ((void*)_pobj) _printobj_6<T1, T2, T3, T4, T5, T6>(fmt, a1, a2, a3, a4, a5, a6);
		}
		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
		mwx_format(const char* fmt, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7) {
			static_assert(sizeof(_printobj_7<T1, T2, T3, T4, T5, T6, T7>) <= MAX_SIZE_PRINTOBJ, "Pre-alloc size overflow. Check MAX_SIZE_PRINTOBJ.");
			(void)new ((void*)_pobj) _printobj_7<T1, T2, T3, T4, T5, T6, T7>(fmt, a1, a2, a3, a4, a5, a6, a7);
		}
		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
		mwx_format(const char* fmt, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8) {
			static_assert(sizeof(_printobj_8<T1, T2, T3, T4, T5, T6, T7, T8>) <= MAX_SIZE_PRINTOBJ, "Pre-alloc size overflow. Check MAX_SIZE_PRINTOBJ.");
			(void)new ((void*)_pobj) _printobj_8<T1, T2, T3, T4, T5, T6, T7, T8>(fmt, a1, a2, a3, a4, a5, a6, a7, a8);
		}

        void operator ()(tfcOutput fn, void *pvContext) {
			void *pv = (void*)_pobj;
			reinterpret_cast<_printobj*>(pv)->do_print(fn, pvContext);
		}
	};


	class MWX_Stream_Flush {};
    class MWX_Stream_EndLine {};

	inline namespace L2 {
		extern MWX_Stream_EndLine crlf;
		extern MWX_Stream_Flush flush;
	}

	struct bytelist {
		std::initializer_list<int> _l;	
		bytelist(std::initializer_list<int> &&l) : _l(l) {}
	};
	
	struct null_stream {
		int _i;
		null_stream(size_t i = 1) : _i(i) { if (_i == 0) _i = 1; }
	};

	template <typename T>
	struct bigendian {
		int _v[sizeof(T)];
		
		// for stream output
		bigendian(T v = 0) {
			for (int i = sizeof(T); i != 0; i--) {
				_v[i-1] = v & 0xFF;
				v >>= 8;
			}
		}

		// for stream input
		T get_value() {
			T v = 0;
			for (unsigned i = 0; i < sizeof(T); i++) {
				v <<= 8;
				v |= _v[i];
			}

			return v;
		}

		inline int* begin() { return _v; }
		inline int* end() { return _v + sizeof(T); }
	};

	template <class D>
	class stream {
		typedef void (*tfcOutput)(char character, void* arg);

	protected:
		uint8_t _bSetup;
		uint8_t _bReady;
		uint8_t _bError;
		uint8_t _timeout_centi_sec;
		void* pvOutputContext;

		static const uint8_t _timeout_default = 10;

	public:
		inline D* get_Derived() { return static_cast<D*>(this); }

		stream() : _bReady(0), _bSetup(0), _bError(0), _timeout_centi_sec(_timeout_default), pvOutputContext(nullptr) {}

		// inline operator bool() { return _bReady; }
		inline bool _setup_finished() { return _bSetup; }

		struct _value_type_s {
			int32_t val;
			_value_type_s(int32_t v) : val(v) {}
		};

		struct _value_type_u {
			uint32_t val;
			_value_type_u(uint32_t v) : val(v) {}
		};

		size_t _print(_value_type_s &&v, uint8_t nFormat, bool ln = false) {
			const char f[][4] = { "%d", "%b", "%d", "%o", "%X" };
			int i = 0;
			switch (nFormat) {
			case BIN: i = 1; break;
			case OCT: i = 2; break;
			case DEC: i = 3; break;
			case HEX: i = 4; break;
			}
			size_t sz = (size_t)fctprintf(get_pfcOutout(), pvOutputContext, f[i], v.val);
			if (ln) sz += println();
			return sz;
		}
		size_t print(char val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat); }
		size_t print(int8_t val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat);  }
		size_t print(int16_t val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat); }
		size_t print(int32_t val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat); }
		size_t println(char val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }
		size_t println(int8_t val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }
		size_t println(int16_t val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }
		size_t println(int32_t val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }
#if !defined(TWE_STDINOUT_ONLY)
		size_t println(int val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }
		size_t print(int val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat); }
#endif
		
		size_t _print(const char *str) {
			size_t ct = 0;
			while(*str != 0) {
				get_Derived()->write(*str);
				++str;
				++ct;
			}

			return ct;
		}

		size_t _print(_value_type_u v, int nFormat = DEC, bool ln = false) {
			const char f[][4] = { "%u", "%b", "%u", "%o", "%X" };
			int i = 0;
			switch (nFormat) {
			case BIN: i = 1; break;
			case OCT: i = 2; break;
			case DEC: i = 3; break;
			case HEX: i = 4; break;
			}
			size_t sz =  (size_t)fctprintf(get_pfcOutout(), pvOutputContext, f[i], v.val);
			if (ln) sz += println();
			return sz;
		}
		
		//size_t print(byte val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat); }
		size_t print(uint8_t val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat); }
		size_t print(uint16_t val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat); }
		size_t print(uint32_t val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat); }
		size_t println(uint8_t val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat, true); }
		size_t println(uint16_t val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat, true); }
		size_t println(uint32_t val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat, true); }
#if !defined(TWE_STDINOUT_ONLY)
		size_t print(unsigned val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat); }
		size_t println(unsigned val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat, true); }
#endif

		size_t print(double val, int fix = 2) {
			char f[8] = "%.0f";
			
			if (fix > 9) fix = 9;
			f[2] += fix;

			return (size_t)fctprintf(get_pfcOutout(), pvOutputContext, f, val);
		}

		size_t println(double val, int fix = 2) {
			size_t sz = print(val, fix);
			sz += println();
			return sz;
		}

		size_t print(const char* msg) {
			return _print(msg);
		}

		template <int S>
		size_t print(const char(&msg)[S]) {
			return print((const char*)&msg[0]);
		}

		/**
		 * initializar list 
		 *  assume always uint8_t
		 */
		template <typename T>
		size_t print(std::initializer_list<T> &&l) {
			auto derived = get_Derived();
			for (auto&& x : l) { derived->write(x & 0xFF); }
			return l.size();
		}

		size_t println(const char* msg) {
			size_t siz = _print(msg);
			println();
			return siz + 2;
		}

		template <int S>
		size_t println(const char(&msg)[S]) {
			return println((const char*)&msg[0]);
		}

		size_t println() {
			get_Derived()->write('\r');
			get_Derived()->write('\n');
			return 2;
		}

		inline void putchar(char c) {
			get_Derived()->write(c);
		}

		size_t printfmt(const char* format, ...) {
			va_list va;
			va_start(va, format);
			size_t ret = (size_t)fctvprintf(get_pfcOutout(), pvOutputContext, format, va);
			va_end(va);
			return ret;
		}

		inline void flush() {
			get_Derived()->flush();
		}

		inline D& operator << (const char* msg) {
			_print(msg);
			return *get_Derived();
		}

		inline D& operator << (char c) {
			get_Derived()->write(c);
			return *get_Derived();
		}

		inline D& operator << (const uint8_t* msg) {
			_print((const char *)msg);
			return *get_Derived();
		}

		template <int S>
		inline D& operator << (const uint8_t(&v)[S]) {
			for (int i = 0; i < S; i++) get_Derived()->write(uint8_t(v[i]));
			return *get_Derived();
		}

		inline D& operator << (uint8_t c) {
			get_Derived()->write(c);
			return *get_Derived();
		}

		inline D& operator << (uint32_t v) {
			get_Derived()->write((v >> 24) & 0xFF);
			get_Derived()->write((v >> 16) & 0xFF);
			get_Derived()->write((v >> 8) & 0xFF);
			get_Derived()->write((v >> 0) & 0xFF);
			return *get_Derived();
		}

		inline D& operator << (uint16_t v) {
			get_Derived()->write((v >> 8) & 0xFF);
			get_Derived()->write((v >> 0) & 0xFF);
			return *get_Derived();
		}

		inline D& operator << (int i) {
			auto ret = fctprintf(get_pfcOutout(), pvOutputContext, "%d", i); (void)ret;
			return *get_Derived();
		}

		inline D& operator << (double d) {
			auto ret = fctprintf(get_pfcOutout(), pvOutputContext, "%.2f", d); (void)ret;
			return *get_Derived();
		}

		inline D& operator << (const mwx::_div_chars&& divc) {
			for(auto x : divc) { get_Derived()->write(x); }
			return *get_Derived();
		}
		
		inline D& operator << (const mwx::div_result_i32&& divr) {
			auto divc = divr.format();
			for(auto x : divc) { get_Derived()->write(x); }
			return *get_Derived();
		}
		
		inline D& operator << (const mwx::div_result_i32& divr) {
			auto divc = divr.format();
			for(auto x : divc) { get_Derived()->write(x); }
			return *get_Derived();
		}

		inline D& operator << (mwx::mwx_format&& prt) {
			prt(get_pfcOutout(), pvOutputContext);
			return *get_Derived();
		}

		/**
		 * for mwx::endl
		 */
		inline D& operator << (mwx::MWX_Stream_EndLine& e) {
			get_Derived()->write('\r');
			get_Derived()->write('\n');
			return *get_Derived();
		}


		/**
		 * for mwx::flush
		 */
		inline D& operator << (mwx::MWX_Stream_Flush& e) {
			get_Derived()->flush();
			return *get_Derived();
		}


		/**
		 * to output integer value as bigendian order.
		 */
		template <typename T>
		inline D& operator << (mwx::bigendian<T>&& v) {
			auto derived = get_Derived();
			for (auto&& x : v) {
				derived->write(x);
			}
			return *get_Derived();
		}


		/**
		 * output integer type array specifying begin/end iterator.
		 */
		template <typename T>
		inline D& operator << (std::pair<T*, T*>&& t) {
			auto derived = get_Derived();
#if 1
			auto p = std::get<0>(t);
			auto e = std::get<1>(t);
			for (; p != e; ++p) {
				derived->write(int(*p));
			}
#else
			// same binary size (for_each and lambda)
			std::for_each(std::get<0>(t),std::get<1>(t),[derived](T& x) { derived->write(int(x)); });
#endif

			return *get_Derived();
		}

		template <typename T>
		inline D& operator << (std::pair<T*, int>&& t) {
			auto derived = get_Derived();
			auto p = std::get<0>(t);
			auto e = p + std::get<1>(t);
			for (; p != e; ++p) {
				derived->write(int(*p));
			}

			return *get_Derived();
		}

		/**
		 * wrapping initializer list 
		 */
		inline D& operator << (bytelist&& bl) {
			auto derived = get_Derived();
			for(auto&& x : bl._l) derived->write(x & 0xFF);
			return *get_Derived();
		}

	public: // error handling section
		// error
		uint8_t get_error_status() { return _bError; }
		void clear_error_status() { _bError = 0;  }
		void set_timeout(uint8_t centisec) { _timeout_centi_sec = centisec; }

	private: // stream to object (>> operator)
		inline int _read_byte(uint32_t t_now) {
			auto derived = get_Derived();
			while (1) {
				// timeout check
				if (_timeout_centi_sec != 0xFF && (millis() - t_now > _timeout_centi_sec * 10)) {
					_bError = 1;
					return -1;
				}

				int c = derived->read();
				if (c == -1) continue;

				return c & 0xFF;
			}
		}

		/**
		 * to output integer value as bigendian order.
		 */
		template <typename C>
		inline void _output_cnt(C& v) {
			//auto derived = get_Derived();
			uint32_t t_now = millis();

			for (auto&& x : v) {
				// blocking
				int c = _read_byte(t_now);
				if (c == -1) return;

				x = c & 0xFF;
			}
			return;
		}

	public: // stream to object (>> operator)
		inline D& operator >> (uint32_t& v) {
			mwx::bigendian<uint32_t> b;
			(*this) >> b;
			v = b.get_value();
			return *get_Derived();
		}

		inline D& operator >> (uint16_t& v) {
			mwx::bigendian<uint16_t> b;
			(*this) >> b;
			v = b.get_value();
			return *get_Derived();
		}

		inline D& operator >> (uint8_t& v) {
			uint32_t t_now = millis();
			int r = _read_byte(t_now);
			v = r & 0xFF;
			return *get_Derived();
		}

		inline D& operator >> (char_t& v) {
			uint32_t t_now = millis();
			int r = _read_byte(t_now);
			v = r & 0xFF;
			return *get_Derived();
		}
		
		template <int S>
		inline D& operator >> (uint8_t(&v)[S]) {
			_output_cnt(v);
			return *get_Derived();
		}

		template <typename T>
		inline D& operator >> (mwx::bigendian<T>& v) {
			_output_cnt(v);
			return *get_Derived();
		}

		// discard a byte
		inline D& operator >> (mwx::null_stream&& p) {
			uint32_t t_now = millis();
			while (p._i) {
				int r = _read_byte(t_now);
				if (r == -1) break;

				--p._i;
			}
			return *get_Derived();
		}

	public: // for printf
		/**
		 * @fn	inline void* stream::get_pvOutputContext()
		 *
		 * @brief	for printf
		 *
		 * @returns	Null if it fails, else the pv output context.
		 */
		inline void* get_pvOutputContext() { return pvOutputContext; }

		/**
		 * @brief Set the pvOutputContext object
		 * 
		 * @param p object which implements 
		 */
		inline void set_pvOutputContext(void *p) { pvOutputContext = p; }

		/**
		 * @fn	inline tfcOutput stream::get_pfcOutout()
		 *
		 * @brief	for printf
		 *
		 * @returns	The pfc outout.
		 */
		inline tfcOutput get_pfcOutout() { return get_Derived()->vOutput; }
	};

	template <class T>
	class _stream_helper_array_type {
	protected:
		T* _ref;
		int _idx_rw;

	public:
		// _stream_helper_array_type() : _ref(nullptr), _idx_rw(0) {}

		// normal constructor
		_stream_helper_array_type(T& ref) : _ref(&ref), _idx_rw(0) {}
		// move constructor
		_stream_helper_array_type(_stream_helper_array_type&& ref) : _ref(ref._ref), _idx_rw(ref._idx_rw) {}

		/**
		 * @brief rewind the read index to the head.
		 */
		inline void rewind() { _idx_rw = 0; }

		/**
		 * @brief set the position.
		 */
		inline int seek(int offset, int whence = MWX_SEEK_SET) { 
			switch (whence) {
				case MWX_SEEK_END: _idx_rw = _ref->size() + offset; break;
				case MWX_SEEK_CUR: _idx_rw += offset; break;
				case MWX_SEEK_SET:
				default:
					_idx_rw = offset;
			}
			if (_idx_rw < 0) _idx_rw = 0;
			if (_idx_rw > _ref->size()) _idx_rw = _ref->size();
			return tell();
		}

		/**
		 * @brief get the position.
		 */
		inline int tell() { return _idx_rw >= _ref->size() ? -1 : _idx_rw; }

		/**
		 * @brief check if there is remaining buffer to read.
		 * 
		 * @return int 0:no data 1:there is data to read
		 */
		inline int available() {
			return (_idx_rw < _ref->size());
		}
	};
}} // L1 // mwx
