/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <utility>

#include "mwx_common.hpp"
#include "mwx_utils_alloc.hpp"
#include "mwx_stream.hpp"

namespace mwx { inline namespace L1 {
	/**
	 * @brief simple array class (mainly for byte array, uint8_t)
	 * 
	 * @tparam T entry type
	 * @tparam AL memory allocator class
	 */
	template <class T, class AL>
	class _smplbuf : public AL
	{
	public:
		typedef _smplbuf self_type;

		typedef T value_type;
		typedef T* iterator;
		typedef const T* const_iterator;

	private:
		uint16_t _u16len;

		/**
		 * @brief copy for initializer `list'
		 * 
		 * @param list 
		 */
		inline void copy_from_initializer_list(std::initializer_list<T>& list) {
			_u16len = (uint16)list.size();
			if (_u16len > AL::super::_size) _u16len = AL::super::_size;

			auto p = list.begin();
			auto q = this->begin();
			while (p != list.end() && q != this->end()) {
				*q++ = *p++;
			}
		}

	public:
		/**
		 * @brief Construct a new smplbuf object
		 * 
		 */
		_smplbuf() : _u16len(0) {}

		/**
		 * @brief Construct a new smplbuf object (alloc_local)
		 * 
		 * @param list the initializer list (e.g. {1,1,2})
		 */
		_smplbuf(std::initializer_list<T>&& list) : _u16len(0)  {
			AL::_is_local();
			init_local();

			if (AL::super::_p != nullptr) {
				copy_from_initializer_list(list);
			}
		}

		/**
		 * @brief Construct a new smplbuf object (alloc attach)
		 * 
		 * @param p pointer to an existing buffer
		 * @param u16len  initial effective length in buffer
		 * @param u16maxlen  max buffer length
		 */
		_smplbuf(T* p, uint16_t u16len, uint16_t u16maxlen) {
			attach(p, u16len, u16maxlen);
		}

		/**
		 * @brief init for alloc_attach<>
		 * 
		 * @param p pointer to an existing buffer
		 * @param l initial effective length in buffer
		 * @param lm max buffer length
		 */
		inline void attach(T* p, uint16_t l, uint16_t lm) {
			AL::_is_attach();
			AL::attach(p, lm);
			_u16len = l;
		}

		/**
		 * @brief init for alloc_local.
		 * 
		 */
		inline void init_local() {
			AL::_is_local();
			AL::init_local();
			_u16len = 0;
		}
		
		/**
		 * @brief init for alloc_heap
		 * 
		 * @param n 
		 */
		inline void init_heap(uint16_t n) {
			AL::_is_heap();
			AL::init_heap(n);
			_u16len = 0;
		}

		/**
		 * @fn	smplbuf::~smplbuf()
		 *
		 * @brief destructor
		 *
		 */
		~_smplbuf() {}

		/**
		 * @brief Construct a new smplbuf object (for alloc_attach)
		 * 
		 * @param ref 
		 */
		_smplbuf(const self_type& ref) {
			static_assert(AL::_type != ALLOC_TYPE_HEAP, "_smplbuf(const self_type&): alloc_heap does not support.");

			if (AL::_type == ALLOC_TYPE_ATTACH) {
				AL::super::_p = ref._p;
				AL::super::_size = ref.capacity();
				_u16len = ref._u16len;
			} else if (AL::_type == ALLOC_TYPE_LOCAL) {
				// copy the items
				_u16len = ref._u16len;
				for (int i = 0; i < ref._u16len; i++) {
					AL::super::_p[i] = ref._p[i];
				}
			}
		}

		/**
		 * @fn	smplbuf& smplbuf::operator= (const smplbuf& ref)
		 *
		 * @brief	assign with same class object. (for alloc_attach)
		 *
		 * @param	ref	.
		 *
		 * @returns	A shallow copy of this object.
		 */
		_smplbuf& operator = (const self_type& ref) {
			static_assert(AL::_type != ALLOC_TYPE_HEAP, "_smplbuf::operator = (const self_type&): alloc_heap does not support.");
			
			if (AL::_type == ALLOC_TYPE_ATTACH) {
				_u16len = ref._u16len;
				AL::super::_p = ref._p;
				AL::super::_size = ref.capacity();
			} else {
				// copy the items
				_u16len = ref._u16len;
				for (int i = 0; i < ref._u16len; i++) {
					AL::super::_p[i] = ref._p[i];
				}
			}
			
			return (*this);
		}

		/**
		 * @brief assing with initializer list (e.g. {0,1,2})
		 * 
		 * @param list the list
		 * @return _smplbuf& 
		 */
		_smplbuf& operator = (std::initializer_list<T>&& list) {
			if (AL::super::_p != nullptr) {
				copy_from_initializer_list(list);
			}
			return (*this);
		}

		/**
		 * @brief get begin iterator
		 * 
		 * @return T* begin iterator (ptr)
		 */
		inline T* begin() { return AL::super::_p; }

		/**
		 * @brief get begin iterator
		 * 
		 * @return T* begin iterator (ptr)
		 */
		inline const T* cbegin() const { return AL::super::_p; }

		/**
		 * @brief get end iterator
		 * 
		 * @return T* end iterator (ptr)
		 */
		inline T* end() { return AL::super::_p + _u16len; }

		/**
		 * @brief get end iterator
		 * 
		 * @return T* end iterator (ptr)
		 */
		inline const T* cend() const { return AL::super::_p + _u16len; }

		/**
		 * @brief append one entry at the end of buffer
		 * 
		 * @param c entry to add
		 * @return true success
		 * @return false buffer full 
		 */
		inline bool append(T&& c) {
			if (_u16len < AL::super::_size) {
				AL::super::_p[_u16len++] = std::move(c);
				return true;
			}
			else {
				return false;
			}
		}

		/**
		 * @brief append one entry at the end of buffer
		 * 
		 * @param c entry to add
		 * @return true success
		 * @return false buffer full 
		 */
		inline bool append(const T& c) {
			if (_u16len < AL::super::_size) {
				AL::super::_p[_u16len++] = c;
				return true;
			}
			else {
				return false;
			}
		}

		/**
		 * @brief append one entry at the end of buffer.
		 * @param c entry to add
		 */
		inline void push_back(T&& c) {
			if (_u16len < AL::super::_size) {
				AL::super::_p[_u16len++] = std::move(c);
			}
		}

		/**
		 * @brief append one entry at the end of buffer.
		 * @param c 
		 */
		inline void push_back(const T& c) {
			if (_u16len < AL::super::_size) {
				AL::super::_p[_u16len++] = c;
			}
		}

		/**
		 * @brief remove the last entry
		 * 
		 */
		inline void pop_back() {
			if (_u16len > 0) {
				AL::super::_p[_u16len - 1] = T{};
				_u16len--;
			}
		}

		/**
		 * @brief check if buffer has no entry.
		 * 
		 * @return true no entry
		 * @return false has entry(ies).
		 */
		inline bool empty() const {
			return (_u16len == 0);
		}

		/**
		 * @brief report entry count
		 * 
		 * @return uint16_t entry count
		 */
		inline uint16_t size() const { return _u16len; }

		/**
		 * @brief report max buffer size
		 * 
		 * @return uint16_t max buffer size
		 */
		inline uint16_t capacity() const { return AL::super::_size; } 

		/**
		 * @brief reserve buffer with given 'len'
		 *        - keep the existing buffer area within 'len'
		 *        - if `len' excesses an existing area, the extended area is filled with blank.
		 * 
		 * @param len length to reserve.
		 * @return true success
		 * @return false not enough buffer size.
		 */
		inline bool reserve(uint16_t len) {
			if (len < AL::super::_size) {
				if (len >= _u16len) {
					// clear the extended area
					T* p = AL::super::_p + _u16len;
					T* e = AL::super::_p + len;
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

		/**
		 * @brief extend/shrink buffer without clearing memory region.
		 * 
		 * @param len new buffer size.
		 */
		inline void redim(uint16_t len) {
			if (len <= AL::super::_size) {
				_u16len = len;
			}
		}


		/**
		 * @brief clear the buffer, set as zero length.
		 */
		inline void clear() {
			_u16len = 0;
		}

		/**
		 * @brief reserve head of buffer by `len'.
		 *        - once reserved, this buffer behaves sub-buffer skipping head area.
		 *        - to recover whole buffer, call .reserve_head(-len).
		 *        note: this operation is NOT safe (e.g. it may destry memory area if calling twice.)
		 * 
		 * @param len 
		 */
		inline void reserve_head(int len) {
			if (len > 0) {
				AL::super::_p += len;
				_u16len = 0;
				AL::super::_size -= len;
			}
			else {
				AL::super::_p += len;
				_u16len += -len;
				AL::super::_size += -len;
			}
		}

		/**
		 * @brief check if it's buffer end or not.
		 * 
		 * @return true filled whole buffer area.
		 * @return false has more space.
		 */
		inline bool is_end() const {
			return (_u16len >= AL::super::_size) ? true : false;
		}

		/**
		 * @brief access an entry by index
		 *        - giving -1 will return the last entryh
		 *        - note: no range check.
		 * 
		 * @param i index
		 * @return T& an entry
		 */
		inline T& operator [] (int i) { return (i < 0) ? AL::super::_p[_u16len + i] : AL::super::_p[i]; }
		
		/**
		 * @brief access an entry by index (const)
		 *        - giving -1 will return the last entryh
		 *        - no range check.
		 * 
		 * @param i index
		 * @return T& an entry
		 */
		inline T operator [] (int i) const { return (i < 0) ? AL::super::_p[_u16len + i] : AL::super::_p[i]; }

		/**
		 * @fn	inline std::pair<T*, T*> smplbuf::to_stream()
		 *
		 * @brief	pair of begin() and end() pointers to output bytes via mwx::stream
		 *
		 * @returns	This object as a std::tuple&lt;T*,T*&gt;
		 */
		inline std::pair<T*, T*> to_stream() {
			static_assert(sizeof(T) == 1, "smplbuf::to_stream() does not support types other than bytetype(char,uint8_t,..).");
			return std::make_pair(this->begin(), this->end());
		}

		/**
		 * @brief helper class with mwx::stream interface.
		 *          smplbuf_u8<128> buff;
		 *          auto&& bs = buff.get_stream_helper();
		 *          bs << format("hello world %d", 123);
		 * 			Serial << buff;
		 * 
		 *        note: do not implement mwx::smpl_buf methods.
		 * 
		 */
		class _smplbuf_strm_hlpr : public mwx::stream<_smplbuf_strm_hlpr>, public mwx::_stream_helper_array_type<_smplbuf> {
			using SUPER_STREAM = mwx::stream<_smplbuf_strm_hlpr>;
			using SUPER_HLPR = mwx::_stream_helper_array_type<_smplbuf>;
			using self_type = _smplbuf_strm_hlpr;

			/**
			 * @brief Set the this object pointer to stream class.
			 *        (note: this is used for calling printf_ library in C)
			 */
			inline void set_obj() {
				SUPER_STREAM::set_pvOutputContext(static_cast<self_type*>(this));
			}

		public: // override constructors/assignment operators/initialization methods.
			_smplbuf_strm_hlpr(_smplbuf& ref) : SUPER_HLPR(ref)  {
				set_obj();
			}

			// handles move constructor
			_smplbuf_strm_hlpr(self_type&& ref) : SUPER_HLPR(std::forward<self_type>(ref)) {
				set_obj();
			}

			// deletes copy constructor
			_smplbuf_strm_hlpr(const self_type& ref) = delete;

		public: // implement stream interfacee
			/**
			 * @brief read a byte from buffer.
			 * 
			 * @return int -1:error or read byte
			 */
			inline int read() { 
				return SUPER_HLPR::available() ? (*SUPER_HLPR::_ref)[SUPER_HLPR::_idx_rw++] : -1;
			}

			/**
			 * @brief write at the pos or append it.
			 * 
			 * @param n 
			 * @return size_t 
			 */
			inline size_t write(int n) {
				int ret;
				// append a byte
				if (SUPER_HLPR::_idx_rw >= SUPER_HLPR::_ref->size()) {
					// append at the end
					ret = SUPER_HLPR::_ref->append(T(n));
					SUPER_HLPR::_idx_rw = SUPER_HLPR::_ref->size(); // at the end
				} else {
					// over write
					(*SUPER_HLPR::_ref)[SUPER_HLPR::_idx_rw++] = T(n);
					ret = 1;
				}
					
				return ret ? 1 : 0;
			}

			/**
			 * @brief flush it, do nothing
			 */
			inline void flush(void) { } // do nothing

			/**
			 * @brief output function for printf_ library.
			 * 
			 * @param out 
			 * @param vp 
			 */
			inline static void vOutput(char out, void* vp) {
				// append a byte
				if (vp != nullptr) {
					auto thisobj = reinterpret_cast<self_type*>(vp);
					thisobj->write(T(out));
				}
			}

			/**
			 * @fn	inline std::pair<T*, T*> smplbuf::to_stream()
			 *
			 * @brief	pair of begin() and end() pointers to output bytes via mwx::stream
			 *
			 * @returns	This object as a std::tuple&lt;T*,T*&gt;
			 */
			inline std::pair<T*, T*> to_stream() {
				static_assert(sizeof(T) == 1, "smplbuf::to_stream() does not support types other than bytetype(char,uint8_t,..).");
				return std::make_pair(SUPER_HLPR::_ref->begin(), SUPER_HLPR::_ref->end());
			}
		};

		/**
		 * @brief	generate helper class with stream interface.
		 */
		_smplbuf_strm_hlpr get_stream_helper() {
			return std::move(_smplbuf_strm_hlpr(*this));
		}

	};

	/**
	 * @brief simple array class with stream interface.
	 * 
	 * @tparam T an entry (note: this class assumes uint8_t)
	 * @tparam AL memory allocator
	 */
	template <typename T, class AL>
	class _smplbuf_stream : public _smplbuf<T, AL>, public mwx::stream<_smplbuf_stream<T, AL>> {
		using SUPER_SMPLBUF = _smplbuf<T, AL>;
		using SUPER_STREAM = mwx::stream<_smplbuf_stream<T, AL>>;
		using self_type = _smplbuf_stream;

		/**
		 * @brief Set the this object pointer to stream class.
		 *        (note: this is used for calling printf_ library in C)
		 */
		inline void set_obj() {
			SUPER_STREAM::set_pvOutputContext(static_cast<self_type*>(this));
		}

	public: // override constructors/assignment operators/initialization methods.
		_smplbuf_stream() : SUPER_SMPLBUF() {
			set_obj();
		}

		_smplbuf_stream(const SUPER_SMPLBUF& ref) : SUPER_SMPLBUF::_smplbuf_stream(ref) {
			set_obj();
		}

		_smplbuf_stream& operator = (const SUPER_SMPLBUF& ref) {
			SUPER_SMPLBUF::operator = (ref);
			set_obj();

			return *this;
		}

		inline void attach(T* p, uint16_t l, uint16_t lm) {
			SUPER_SMPLBUF::attach(p, l, lm);
			set_obj();
		}

		inline void init_local() {
			SUPER_SMPLBUF::init_local();
			set_obj();
		}

		inline void init_heap(uint16_t n) {
			SUPER_SMPLBUF::init_heap(n);
			set_obj();
		}

	public: // implement stream interfacee
		inline int available() { return 0; } // always false (read() is not implemented)
		inline int read() { return -1; } // not implemented
		inline void flush(void) {} // do nothing

		// append one entry
		inline size_t write(int n) {
			// append a byte
			bool ret = SUPER_SMPLBUF::append((T)n);
			return ret ? 1 : 0;
		}

		// output function for printf_ library.
		inline static void vOutput(char out, void* vp) {
			// append a byte
			if (vp != nullptr) {
				auto thisobj = reinterpret_cast<self_type*>(vp);
				thisobj->append((T)out);
			}
		}

		/**
		 * @fn	inline std::pair<T*, T*> smplbuf::to_stream()
		 *
		 * @brief	pair of begin() and end() pointers to output bytes via mwx::stream
		 *
		 * @returns	This object as a std::tuple&lt;T*,T*&gt;
		 */
		inline std::pair<T*, T*> to_stream() {
			static_assert(sizeof(T) == 1, "smplbuf::to_stream() does not support types other than bytetype(char,uint8_t,..).");
			return std::make_pair(SUPER_SMPLBUF::begin(), SUPER_SMPLBUF::end());
		}
	};

	// alias _smplbuf to smplbuf
	template <typename T, class AL = mwx::alloc_attach<T>>
	using smplbuf = mwx::_smplbuf<T, AL>;

	using smplbuf_u8_attach = mwx::smplbuf<uint8_t, mwx::alloc_attach<uint8_t>>;
	template <int N> using smplbuf_u8 =  mwx::smplbuf<uint8_t, mwx::alloc_local<uint8_t, N>>;
	using smplbuf_u8_heap = mwx::smplbuf<uint8_t, mwx::alloc_heap<uint8_t>>;

	using smplbuf_strm_u8_attach = mwx::_smplbuf_stream<uint8_t, mwx::alloc_attach<uint8_t>>;
	template <int N> using smplbuf_strm_u8 = _smplbuf_stream<uint8_t, mwx::alloc_local<uint8_t, N>>;
	using smplbuf_strm_u8_heap = mwx::_smplbuf_stream<uint8_t, mwx::alloc_heap<uint8_t>>;

	template <typename T, int N>
	using smplbuf_local =  mwx::smplbuf<T, mwx::alloc_local<T, N>>;
	template <typename T>
	using smplbuf_attach =  mwx::smplbuf<T, mwx::alloc_attach<T>>;
	template <typename T>
	using smplbuf_heap =  mwx::smplbuf<T, mwx::alloc_heap<T>>;

	/**
	 * @brief Output bytes from RHS object of smplbuf_stream<uint8_t> with stream interface into LHS object of mwx::stream<uint8_t>.
	 * 
	 * @tparam L_STRM 
	 * @tparam ALC 
	 * @param lhs 
	 * @param rhs 
	 * @return mwx::stream<L_STRM>& 
	 */
	template <class L_STRM, class AL>
	mwx::stream<L_STRM>& operator << (mwx::stream<L_STRM>& lhs, mwx::_smplbuf_stream<uint8_t, AL>& rhs) {
		lhs << rhs.to_stream();
		return lhs;
	}

	/**
	 * @brief Output bytes from LHS object of smplbuf<uint8_t> into RHS object of mwx::stream<uint8_t>.
	 * 
	 * @tparam L_STRM 
	 * @tparam ALC 
	 * @param lhs 
	 * @param rhs 
	 * @return mwx::stream<L_STRM>& 
	 */
	template <class L_STRM, class AL>
	mwx::stream<L_STRM>& operator << (mwx::stream<L_STRM>& lhs, mwx::_smplbuf<uint8_t, AL>& rhs) {
		for (uint8_t x : rhs) lhs << uint8_t(x);
		return lhs;
	}

	/**
	 * @brief Put RHS content of smplbuf_stream<uint8_t> into LHS content of smplbuf_stream<uint8_t>.
	 * 
	 * @tparam L_ALC 
	 * @tparam R_ALC 
	 * @param lhs 
	 * @param rhs 
	 * @return mwx::_smplbuf_stream<uint8_t, L_ALC>& 
	 */
	template <class L_AL, class R_AL>
	mwx::_smplbuf_stream<uint8_t, L_AL>& operator << (mwx::_smplbuf_stream<uint8_t, L_AL>& lhs, mwx::_smplbuf_stream<uint8_t, R_AL>& rhs) {
		for (uint8_t x : rhs) lhs.push_back(x);
		return lhs;
	}

	/**
	 * @brief Put RHS content of smplbuf<uint8_t> into LHS content of smplbuf_stream<uint8_t>. 
	 * 
	 * @tparam L_ALC 
	 * @tparam R_ALC 
	 * @param lhs 
	 * @param rhs 
	 * @return mwx::_smplbuf_stream<uint8_t, L_ALC>& 
	 */
	template <class L_AL, class R_AL>
	mwx::_smplbuf_stream<uint8_t, L_AL>& operator << (mwx::_smplbuf_stream<uint8_t, L_AL>& lhs, mwx::_smplbuf<uint8_t, R_AL>& rhs) {
		for (uint8_t x : rhs) lhs.push_back(x);
		return lhs;
	}

	
}} // TWEUTILS
