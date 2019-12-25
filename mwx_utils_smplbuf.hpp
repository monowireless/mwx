/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <tuple>

// #include "twecommon.h"
#include "mwx_common.hpp"
#include "mwx_utils_alloc.hpp"
#include "mwx_stream.hpp"

namespace mwx { inline namespace L1 {
	/**
	 * @class	smplbuf
	 *
	 * @brief	簡易バッファ
	 * 			
	 * 			- このクラス自体はメモリの確保をせず、あらかじめ確保済みのメモリ領域を配列としてアクセスする  
	 * 			  ための手続きを提供する。
	 * 			- 確保された最大長の範囲で可変長のバッファとしてふるまう。redim() により利用範囲を変更する。
	 *
	 * @tparam	T	Generic type parameter.
	 * @tparam	N	Type of the n.
	 */
	template <class T, class alloc = mwx::alloc_attach<T>>
	class smplbuf : public alloc, public mwx::stream<smplbuf<T,alloc>>
	{
	public:
		typedef smplbuf self_type;

		typedef T value_type;
		typedef T* iterator;
		typedef const T* const_iterator;
		
		using SUPER_STREAM = mwx::stream<smplbuf<T,alloc>>;
		
	private:
		uint16_t _u16len;

		/// <summary>
		/// initializer list からのコピー
		/// </summary>
		/// <param name="list"></param>
		inline void copy_from_initializer_list(std::initializer_list<T>& list) {
			_u16len = (uint16)list.size();
			if (_u16len > alloc::super::_size) _u16len = alloc::super::_size;

			auto p = list.begin();
			auto q = this->begin();
			while (p != list.end() && q != this->end()) {
				*q++ = *p++;
			}
		}

	public:
		/// <summary>
		/// コンストラクタ
		/// </summary>
		smplbuf() : _u16len(0) {
			SUPER_STREAM::pvOutputContext = (void*)this;
		}

		smplbuf(std::initializer_list<T>&& list) : _u16len(0)  {
			alloc::_is_local();
			init_local();

			if (alloc::super::_p != nullptr) {
				copy_from_initializer_list(list);
			}
		}

		/**
		 * @fn	smplbuf::smplbuf(T* p, uint16_t u16len, uint16_t u16maxlen)
		 *
		 * @brief	コンストラクタ、パラメータ全部渡し
		 *
		 * @param [in,out]	p		 	.
		 * @param 		  	u16len   	.
		 * @param 		  	u16maxlen	.
		 */
		smplbuf(T* p, uint16_t u16len, uint16_t u16maxlen) {
			attach(p, u16len, u16maxlen);
		}


		inline void attach(T* p, uint16_t l, uint16_t lm) {
			alloc::_is_attach();
			alloc::attach(p, lm);
			_u16len = l;
			SUPER_STREAM::pvOutputContext = (void*)this;
		}

		inline void init_local() {
			alloc::_is_local();
			alloc::init_local();
			_u16len = 0;
			SUPER_STREAM::pvOutputContext = (void*)this;
		}
		
		inline void init_heap(uint16_t n) {
			alloc::_is_heap();
			alloc::init_heap(n);
			_u16len = 0;
			SUPER_STREAM::pvOutputContext = (void*)this;
		}

		/**
		 * @fn	smplbuf::~smplbuf()
		 *
		 * @brief	デストラクタ
		 *
		 */
		~smplbuf() {}

		/**
		 * @fn	smplbuf::smplbuf(const smplbuf& ref)
		 *
		 * @brief	コピーコンストラクタ
		 * 			クラス内にメモリを確保する場合はコピー禁止とする。
		 *
		 * @param	ref	.
		 */
		smplbuf(const self_type& ref) {
			alloc::_is_attach(); // validate if attaching allocator.
			SUPER_STREAM::pvOutputContext = (void*)this;

			alloc::super::_p = ref._p;
			alloc::super::_size = ref.capacity();
			_u16len = ref._u16len;
		}

		/**
		 * @fn	smplbuf& smplbuf::operator= (const smplbuf& ref)
		 *
		 * @brief	代入演算子
		 *
		 * @param	ref	.
		 *
		 * @returns	A shallow copy of this object.
		 */
		smplbuf& operator = (const self_type& ref) {
			alloc::_is_attach(); // validate if attaching allocator.
			SUPER_STREAM::pvOutputContext = (void*)this;

			_u16len = ref._u16len;
			alloc::super::_p = ref._p;
			alloc::super::_size = ref.capacity();
			return (*this);
		}

		smplbuf& operator = (std::initializer_list<T>&& list) {
			if (alloc::super::_p != nullptr) {
				copy_from_initializer_list(list);
			}
			return (*this);
		}

		/// <summary>
		/// 先頭
		/// </summary>
		/// <returns></returns>
		inline T* begin() { return alloc::super::_p; }
		inline const T* cbegin() const { return alloc::super::_p; }

		/// <summary>
		/// 終端（実データ）
		/// </summary>
		/// <returns></returns>
		inline T* end() { return alloc::super::_p + _u16len; }
		inline const T* cend() const { return alloc::super::_p + _u16len; }

		/// <summary>
		/// １バイト追加
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		inline bool append(T&& c) {
			if (_u16len < alloc::super::_size) {
				alloc::super::_p[_u16len++] = c;
				return true;
			}
			else {
				return false;
			}
		}

		inline bool append(const T& c) {
			if (_u16len < alloc::super::_size) {
				alloc::super::_p[_u16len++] = c;
				return true;
			}
			else {
				return false;
			}
		}

		inline void push_back(T&& c) {
			if (_u16len < alloc::super::_size) {
				alloc::super::_p[_u16len++] = c;
			}
		}

		inline void push_back(const T& c) {
			if (_u16len < alloc::super::_size) {
				alloc::super::_p[_u16len++] = c;
			}
		}

		inline void pop_back() {
			if (_u16len > 0) {
				alloc::super::_p[_u16len - 1] = T{};
				_u16len--;
			}
		}

		inline bool empty() const {
			return (_u16len == 0);
		}

		/// <summary>
		/// 配列の有効データ長
		/// </summary>
		/// <returns></returns>
		inline uint16_t size() const { return _u16len; }

		/// <summary>
		/// 配列の最大バッファサイズ
		/// </summary>
		/// <returns></returns>
		inline uint16_t capacity() const { return alloc::super::_size; } 

		/// <summary>
		/// 使用区画のサイズを変更する
		/// </summary>
		/// <param name="len">新しいサイズ</param>
		/// <returns></returns>
		inline bool reserve(uint16_t len) {
			if (len < alloc::super::_size) {
				if (len >= _u16len) {
					// 後ろをクリア
					T* p = alloc::super::_p + _u16len;
					T* e = alloc::super::_p + len;
					while (p < e) {
						(*p) = T{};
						++p;
					}
				}
				_u16len = len;
				return true;
			}
			else {
				return false;
			}
		}

		/// <summary>
		/// 使用区画の先頭部分を予約する
		/// </summary>
		/// <param name="len">予約サイズ</param>
		/// <returns></returns>
		inline void reserve_head(int len) {
			if (len > 0) {
				alloc::super::_p += len;
				_u16len = 0;
				alloc::super::_size -= len;
			}
			else {
				alloc::super::_p += len;
				_u16len += -len;
				alloc::super::_size += -len;
			}
		}

		// バッファ領域をクリアせずにサイズを変更する
		inline void redim(uint16_t len) {
			if (len <= alloc::super::_size) {
				_u16len = len;
			}
		}

		/// <summary>
		/// 終端に来た場合（これ以上追加できない）
		/// </summary>
		/// <returns></returns>
		inline bool is_end() const {
			return (_u16len >= alloc::super::_size) ? true : false;
		}

		/// <summary>
		/// []演算子、インデックスの範囲外チェックはしない。
		///  - (-1)をインデックスに与えると配列の末尾を示す
		/// </summary>
		/// <param name="i"></param>
		/// <returns></returns>
		inline T& operator [] (int i) { return (i < 0) ? alloc::super::_p[_u16len + i] : alloc::super::_p[i]; }
		inline T operator [] (int i) const { return (i < 0) ? alloc::super::_p[_u16len + i] : alloc::super::_p[i]; }

		/// <summary>
		/// 代入演算子(自身の配列の利用長を０に戻す）
		///   redim() にて = 0 にて型 T を初期化している。SmplBuf<T> の場合でも、
		///   このクラスを利用できるようにするための、初期化演算子として利用。
		/// </summary>
		inline self_type& operator = (int i) { _u16len = 0; return *this; }

		// implement stream interfaces
		inline int available() { return 0; }
		inline void flush(void) {
			// set terminator char (note: do not append)
			if (_u16len < alloc::super::_size)
				alloc::super::_p[_u16len] = 0;
			else
				alloc::super::_p[alloc::super::_size - 1] = 0;
		}
		inline int read() { return -1; }
		inline size_t write(int n) {
			// append a byte
			bool ret = append((T)n);
			return ret ? 1 : 0;
		}
		inline static void vOutput(char out, void* vp) {
			// append a byte
			if (vp != nullptr) {
				auto thisobj = reinterpret_cast<self_type*>(vp);
				thisobj->append((T)out);
			}
		}

		/**
		 * @fn	inline std::tuple<T*, T*> smplbuf::to_stream()
		 *
		 * @brief	mwx::stream クラスへ出力するために begin(), end() のペアを返す。
		 *
		 * @returns	This object as a std::tuple&lt;T*,T*&gt;
		 */
		inline std::pair<T*, T*> to_stream() {
			static_assert(sizeof(T) == 1, "smplbuf::to_stream() does not support types other than bytetype(char,uint8_t,..).");
			return std::make_pair(begin(), end());
		}
	};

	using smplbuf_u8_attach = mwx::smplbuf<uint8_t, mwx::alloc_attach<uint8_t>>;
	template <int N> using smplbuf_u8 =  mwx::smplbuf<uint8_t, mwx::alloc_local<uint8_t, N>>;
	using smplbuf_u8_heap = mwx::smplbuf<mwx::alloc_heap<uint8_t>>;

	template <typename T, int N>
	using smplbuf_local =  mwx::smplbuf<T, mwx::alloc_local<T, N>>;
	template <typename T>
	using smplbuf_attach =  mwx::smplbuf<T, mwx::alloc_attach<T>>;
	template <typename T>
	using smplbuf_heap =  mwx::smplbuf<T, mwx::alloc_heap<T>>;


}} // TWEUTILS
