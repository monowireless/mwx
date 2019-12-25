/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#include <cstring>

#include "twecommon.h"
#include "tweserial.h"
#include "twesercmd_gen.h"

#include "mwx_common.hpp"
#include "mwx_debug.h"
#include "mwx_utils_smplbuf.hpp"

namespace mwx {  inline namespace L1 {
	namespace PARSER {
		const uint8_t ASCII = 0x01;
		const uint8_t BINARY = 0x02;
	}

	/**
	 * @class	serial_parser
	 *
	 * @brief	Wrapper class for twesercmd (ASCII/BINARY serial format)
	 * 			- this class has fixed uint8_t array (128bytes) to hold parsed values.
	 */
	template <class alloc>
	class serial_parser {
	public:
		using BUFTYPE = mwx::smplbuf<uint8_t, alloc>;
		
	private:
		TWESERCMD_tsSerCmd_Context _context;
		BUFTYPE _buf;
		uint8_t _type;

	public:
		serial_parser() : _context{ 0 }, _type(0) {}

		// use internal local array
		void begin(uint8_t ty) {
			_buf._is_local();
			_buf.init_local(); // initialize

			_type = ty;
			_init_parser();
		}
		
		// use external buffer
		void begin(uint8_t ty, uint8_t *p, uint16_t siz, uint16_t max_siz) {
			_buf._is_attach();
			_buf.attach(p, siz, max_siz);

			_type = ty;
			_init_parser();

			if (siz != 0) {
				_context.u16len = siz;
			}
		}
		
		// allocate global memory block.
		void begin(uint8_t ty, uint16_t siz) {
			_buf._is_heap();
			_buf.init_heap(siz);

			_type = ty;
			_init_parser();
		}

		void _init_parser() {
			switch (_type) {
			case PARSER::BINARY:
				TWESERCMD_Binary_vInit(&_context, _buf.begin(), _buf.capacity());
				break;
			case PARSER::ASCII:
			default:
				TWESERCMD_Ascii_vInit(&_context, _buf.begin(), _buf.capacity());
				break;
			}
		}

		/**
		 * @fn	operator serial_parser::bool()
		 *
		 * @brief	True if parse is complete.
		 *
		 * @returns	true: complete, false: not yet or error.
		 */
		operator bool() { return _context.u8state == E_TWESERCMD_COMPLETE;  }
		bool available() { return operator bool(); }

		inline bool parse(uint8_t b) {
			uint8_t u8res = _context.u8Parse(&_context, b);
			if (u8res == E_TWESERCMD_COMPLETE) {
				_buf.redim(_context.u16len);
				return true;
			}
			return false;
		}

		/**
		 * @fn	template <class T> static int serial_parser::putc(int c, struct _tsTWETERM_FILE* fp)
		 *
		 * @brief	To interface TWE_tsFILE and stream<T>, which calls stream<T>::putchar().
		 * 			
		 * @tparam	T	Generic type parameter.
		 * @param 		  	c 	output char
		 * @param [in,out]	fp	C style output file structure, especially for stream<T>.
		 *
		 * @returns	An int.
		 */
		template <class T>
		static int putc(int c, TWE_tsFILE* fp) {
			stream<T>* p = reinterpret_cast<stream<T>*>(fp->vpContext_output);
			p->putchar(c);
			return 1;
		}

		/**
		 * @fn	template <class T> stream<T>& serial_parser::operator>> (stream<T>& strm)
		 *
		 * @brief	Output formatted data to the stream.
		 *
		 * @param [in,out]	strm	The stream to output.
		 *
		 * @returns	The stream itself.
		 */
		template <class T>
		stream<T>& operator >> (stream<T>& strm) {
			TWE_tsFILE file;
			memset(&file, 0, sizeof(TWE_tsFILE));

			file.fp_putc = putc<T>;
			file.vpContext_output = &strm;

			_context.vOutput(&_context, &file);

			return strm;
		}

		/**
		 * @fn	BUFTYPE& serial_parser::get_buf()
		 *
		 * @brief	Gets the buffer
		 *
		 * @returns	The buffer.
		 */
		BUFTYPE& get_buf() { return _buf; }
	};

	template <class T, class alloc>
	stream<T>& operator << (stream<T>& lhs, serial_parser<alloc>& rhs) {
		rhs >> lhs;
		return lhs;
	}

	using serparser_attach = mwx::serial_parser<mwx::alloc_attach<uint8_t>>;
	template <int N> using serparser_local =  mwx::serial_parser<mwx::alloc_local<uint8_t, N>>;
	using serparser_heap = mwx::serial_parser<mwx::alloc_heap<uint8_t>>;
}}
