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
#include "tweprintf.h"

#include "mwx_common.hpp"
#include "mwx_periph.hpp"
#include "mwx_debug.h"

// for print(xx, YY);
#define DEC 10
#define HEX 16
#define OCT 8
#ifdef BIN
#undef BIN
#endif
#define BIN 2

namespace mwx { inline namespace L1 {
    /// <summary>
    /// fprintf style stream output class
    ///  - will work on 8/16/32/64bit arguments including double.
    ///  - save the arguments upto 4 items at constructor.
    /// </summary>
    class mwx_format {
        static const size_t MAXARGS = 4;
        const char* _fmt;
        int64_t _argv[MAXARGS];
        int _type;
        int _argc;
        typedef void (*tfcOutput)(char character, void* arg);

		
        // template <typename... Tail> void save_args(float head, Tail&&... tail);
        // template <typename... Tail> void save_args(const char *head, Tail&&... tail);

        // on finish.
        void save_args() {}

        // float should be passed by double.
        template <typename... Tail>
        void save_args(float head, Tail&&... tail) {
            double d = head;
            _argv[_argc] = *(int64_t*)&d;
            _type |= 1 << _argc;

            _argc++;

            // get more parameters recursively, split head and tail.
            save_args(std::forward<Tail>(tail)...);
        }

		// specialize all of char related type (singed char is not defined)
		#define _MWX_FMT_BYTE_TYPE() { _argv[_argc++] = (int64_t)(void*)head; save_args(std::forward<Tail>(tail)...); }
		template <typename... Tail> void save_args(char* head, Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail> void save_args(const char* head, Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail> void save_args(int8_t* head, Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail> void save_args(const int8_t* head, Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail> void save_args(uint8_t* head, Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail> void save_args(const uint8_t* head, Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail, int S> void save_args(char(&head)[S], Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail, int S> void save_args(const char(&head)[S], Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail, int S> void save_args(int8_t(&head)[S], Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail, int S> void save_args(const int8_t(&head)[S], Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail, int S> void save_args(uint8_t(&head)[S], Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }
		template <typename... Tail, int S> void save_args(const uint8_t(&head)[S], Tail&&... tail) { _MWX_FMT_BYTE_TYPE(); }

		template <typename T, typename... Tail>
		void save_args(typename std::enable_if< std::is_integral<T>::value, T >::type head, Tail&&... tail) {
			_argv[_argc++] = head;
			save_args(std::forward<Tail>(tail)...);
		}

        // other type is considered into uint type.
        //   not sure it's complete or not...
        template <typename Head, typename... Tail>
        void save_args(Head&& head, Tail&&... tail)
        {
            static_assert(
                   sizeof(Head) == 8
                || sizeof(Head) == 4
                || sizeof(Head) == 2
                || sizeof(Head) == 1
                , "mwx::fmt::save_args()/Unsupported argument.");

            if (sizeof(Head) == 8) {
                _argv[_argc] = *(uint64_t*)(void*)&head;
                _type |= 1 << _argc;
            }
            else if (sizeof(Head) == 4) {
                _argv[_argc] = (*(uint32_t*)(void*)&head) & 0xFFFFFFFF;
            }
            else if (sizeof(Head) == 2) {
                _argv[_argc] = (*(uint16_t*)(void*)&head) & 0xFFFF;
            }
            else if (sizeof(Head) == 1) {
				_argv[_argc] = (*(uint8_t*)(void*)&head) & 0xFF;
            }
            else {
                // Unknown error (should be 4 or 8)
                _argc = -1;
                return;
            }
            _argc++;

            // get more parameters recursively, split head and tail.
            save_args(std::forward<Tail>(tail)...);
        }

    public:
        // constructor with variable numbers of arguments, using parameter list.
        template <typename... Tail>
        mwx_format(const char* fmt, Tail&&... tail) : _argv{ 0 }, _argc(0), _type(0), _fmt(fmt) {
            static_assert(sizeof...(tail) <= MAXARGS, "mwx::fmt()/MAX arguments count is 4.");

            if (sizeof...(tail)) {
                if (sizeof...(tail) > MAXARGS) {
                    _argc = -1;
                }
                else {
                    save_args(tail...);
                }
            }
        }

        // the output call (hmmm, there would be better ways ;-()
        void operator ()(tfcOutput fn, void *pvContext) {
            if (_argc == -1) {
                fctprintf(fn, pvContext, "(arg err)");
            }
            else {
                // should not pass 32bit or less arguments as 64bit.
                switch (_type) {
                case   0:	fctprintf(fn, pvContext, _fmt,  (int32_t)_argv[0],  (int32_t)_argv[1],  (int32_t)_argv[2],  (int32_t)_argv[3]); break;
                case   1:	fctprintf(fn, pvContext, _fmt,           _argv[0],  (int32_t)_argv[1],  (int32_t)_argv[2],  (int32_t)_argv[3]); break;
                case   2:	fctprintf(fn, pvContext, _fmt,  (int32_t)_argv[0],           _argv[1],  (int32_t)_argv[2],  (int32_t)_argv[3]); break;
                case   3:	fctprintf(fn, pvContext, _fmt,           _argv[0],           _argv[1],  (int32_t)_argv[2],  (int32_t)_argv[3]); break;
                case   4:	fctprintf(fn, pvContext, _fmt,  (int32_t)_argv[0],  (int32_t)_argv[1],           _argv[2],  (int32_t)_argv[3]); break;
                case   5:	fctprintf(fn, pvContext, _fmt,           _argv[0],  (int32_t)_argv[1],           _argv[2],  (int32_t)_argv[3]); break;
                case   6:	fctprintf(fn, pvContext, _fmt,  (int32_t)_argv[0],           _argv[1],           _argv[2],  (int32_t)_argv[3]); break;
                case   7:	fctprintf(fn, pvContext, _fmt,           _argv[0],           _argv[1],           _argv[2],  (int32_t)_argv[3]); break;
                case   8:	fctprintf(fn, pvContext, _fmt,  (int32_t)_argv[0],  (int32_t)_argv[1],  (int32_t)_argv[2],           _argv[3]); break;
                case   9:	fctprintf(fn, pvContext, _fmt,           _argv[0],  (int32_t)_argv[1],  (int32_t)_argv[2],           _argv[3]); break;
                case  10:	fctprintf(fn, pvContext, _fmt,  (int32_t)_argv[0],           _argv[1],  (int32_t)_argv[2],           _argv[3]); break;
                case  11:	fctprintf(fn, pvContext, _fmt,           _argv[0],           _argv[1],  (int32_t)_argv[2],           _argv[3]); break;
                case  12:	fctprintf(fn, pvContext, _fmt,  (int32_t)_argv[0],  (int32_t)_argv[1],           _argv[2],           _argv[3]); break;
                case  13:	fctprintf(fn, pvContext, _fmt,           _argv[0],  (int32_t)_argv[1],           _argv[2],           _argv[3]); break;
                case  14:	fctprintf(fn, pvContext, _fmt,  (int32_t)_argv[0],           _argv[1],           _argv[2],           _argv[3]); break;
                case  15:	fctprintf(fn, pvContext, _fmt,           _argv[0],           _argv[1],           _argv[2],           _argv[3]); break;
                }
            }
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

		inline operator bool() { return _bReady; }
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
		size_t print(int val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat); }
		size_t println(char val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }
		size_t println(int8_t val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }
		size_t println(int16_t val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }
		size_t println(int32_t val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }
		size_t println(int val, int nFormat = DEC) { return _print(_value_type_s(val), nFormat, true); }

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
		size_t print(unsigned val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat); }
		size_t println(uint8_t val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat, true); }
		size_t println(uint16_t val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat, true); }
		size_t println(uint32_t val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat, true); }
		size_t println(unsigned val, int nFormat = DEC) { return _print(_value_type_u(val), nFormat, true); }

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
			(size_t)fctprintf(get_pfcOutout(), pvOutputContext, "%d", i);
			return *get_Derived();
		}

		inline D& operator << (double d) {
			(size_t)fctprintf(get_pfcOutout(), pvOutputContext, "%.2f", d);
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
				if (millis() - t_now > _timeout_centi_sec * 10) {
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
		 * @fn	inline tfcOutput stream::get_pfcOutout()
		 *
		 * @brief	for printf
		 *
		 * @returns	The pfc outout.
		 */
		inline tfcOutput get_pfcOutout() { return get_Derived()->vOutput; }
	};
}}
