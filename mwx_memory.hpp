/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

/**
 * Allocates a block of memory from the heap.
 * 
 * @param   pvPointer       if value is assigned, use old one. (normally set NULL)
 * @param   u32BytesNeeded  necessary bytes
 * @param   bClear          clear area with 0.
 * @returns                 allocated memory address.
 */
extern "C" void *pvHeap_Alloc(void *pvPointer, uint32 u32BytesNeeded, bool_t bClear);

/**
 * Initialise the remaining free space with a check character
 *
 * Designed to be called after everything has been allocated.
 * Used to see how close the heap is to running out of memory.
 * Used in conjunction with iHeap_ReturnNumberOfOverwrittenBytes() to see
 * if the free area of the heap is getting overwritten by the stack and thus get
 * an early warning that the stack may be overwriting the heap and causing a crash.
 * 
 * @returns The number of remaining free bytes on the heap.
 */
extern "C" int iHeap_InitialiseOverwriteCheck();

 /**
  * Checks how many bytes of the free heap space have been overwritten.
  *
  * Designed to be called after everything has been allocated.
  * Used to see how close the heap is to running out of memory.
  * Used in conjunction with iHeap_ReturnNumberOfOverwrittenBytes() to see
  * if the free area of the heap is getting overwritten by the stack and thus get
  * an early warning that the stack may be overwriting the heap and causing a crash.
  * 
  * NOTES:
  * Following a call to iHeap_InitialiseOverwriteCheck,
  * Checks how many bytes of the free heap space have been overwritten
  * from the end of the free space.
  *
  * Checks from start of free space until it finds a bad byte.
  * This means it will immediately find a changed byte and
  * return the total free space if anything has been allocated
  * from the heap after the call to iHeap_InitialiseOverwriteCheck.
  * 
  * @returns The number of bytes that have been overwritten by a stack overflow.
  */
extern "C" int iHeap_ReturnNumberOfOverwrittenBytes();

extern "C" uint32_t u32HeapStart;
extern "C" uint32_t u32HeapEnd;

#if 0
extern "C"
{
	extern void (**__init_array_start)();
	extern void (**__init_array_end)();

	void mwx_static_init();
}

extern void* (*_twe_vp_malloc)(size_t size);
extern void (*_twe_vp_free)(void *ptr);
extern void* _twe_malloc_obj;

/**
 * Very very simple allocator, but NEVER free().
 * 
 * This allocator is intended to make single object chosen from classes,
 * which has rather bigger memory usage.
 * 
 * class class1 { ... }; // with constructor
 * class class2 { ... };
 * ...
 * class classn { ... };
 * 
 * ...
 * void *the_obj;
 * if ( ... ) {
 *   obj = (void*)(new class1());
 * } else if ( ...) {
 *   obj = (void*)(new class2());
 * } 
 * 
 * the reasons are:
 *   - if declaring all instances, they consume much of memory.
 *   - use of union is not allowed when members have constructor/destructor.
 * 
 * Usage:
 *   1. declare this object at global area.
 *     memory_nofree<1024> the_memory;
 *   2. call begin()
 *     the_memory.begin();
 *   3. use new!
 *     auto *p = new class1();
 */
namespace MWX {
	template <size_t SIZ = 256>
	class memory_nofree {
		void* _twe_malloc_obj_prev;

		uint8_t _b[SIZ];
		size_t _i;
	public:
		memory_nofree() : _i(0), _twe_malloc_obj_prev(nullptr) {
			_b[0] = 0;
		}

		~memory_nofree() {
			_twe_malloc_obj = _twe_malloc_obj_prev;
		}

		/**
		 * Registers this class object and functions.
		 * now new operator works.
		 */
		void begin() {
			_twe_malloc_obj_prev = _twe_malloc_obj;
			_twe_malloc_obj = (void*)this;
			_twe_vp_malloc = &twe_malloc;
			_twe_vp_free = &twe_free;
		}

		/**
		 * allocate desired size of memory.
		 */
		void* alloc(size_t size) {
			void* ptr = nullptr;
			if (_i + size < SIZ) {
				ptr = (void*)(&_b[_i]);

				_i += (size / 4) * 4; // alignment 32bit
				if (size & 0x3) _i += 4;
			}
			return ptr;
		}

		void free(void* ptr) {
			// DO NOTHING!!!
		}

		static void* twe_malloc(size_t size) {
			if (_twe_malloc_obj != nullptr) {
				auto p = static_cast<memory_nofree*>(_twe_malloc_obj);
				return p->alloc(size);
			}
		}

		static void twe_free(void* ptr) {
			if (_twe_malloc_obj != nullptr) {
				auto p = static_cast<memory_nofree*>(_twe_malloc_obj);
				return p->free(ptr);
			}
		}
	};
}
#endif
