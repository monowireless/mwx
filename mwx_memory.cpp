/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include <cstdint>
#include <cstring>
//#include <new>

#include "ToCoNet.h"
#include "mwx_debug.h"
#include "mwx_memory.hpp"

#if 0 // ndef MWX_NO_NEW_OPERATOR
void* (*_twe_vp_malloc)(size_t size) = nullptr;
void (*_twe_vp_free)(void *ptr) = nullptr;
void* _twe_malloc_obj = nullptr;

void* operator new(size_t size) { 
    if (_twe_vp_malloc != nullptr) {
        void *ptr = _twe_vp_malloc(size);
        MWX_DebugMsg(5, "{NEW(%X):Sz=%d:%X}", (void*)_twe_vp_malloc, size, ptr);
        return ptr;
    } else {
        MWX_DebugMsg(5, "{NEW:NOHNDL}");
        return (void*)0xdeadbeef;
        // return nullptr; // may cause an error
    }
}

void operator delete(void* ptr) {
    if (_twe_vp_free != nullptr) {
        MWX_DebugMsg(5, "{DEL(%X):%X}", (void*)_twe_vp_malloc, ptr);
        return _twe_vp_free(ptr);
    }
}
#endif

void* operator new(size_t size) noexcept {
    if (u32HeapStart + size > u32HeapEnd) {
        //MWX_DebugMsg(0, "{pvHeap_Alloc() fails}");
        return (void*)0xdeadbeef;
    } else {
        void *blk = pvHeap_Alloc(NULL, size, 0);   

		// MWX_DebugMsg(0, "{pvHeap_Alloc(%d)=%x}", size, blk);
        return blk;
    }
}


void* operator new[](size_t size) noexcept {
    return operator new(size);
}

void operator delete(void* ptr) noexcept {
    ; // unfortunately, do nothing
}

void operator delete[](void* ptr) noexcept {
    ; // unfortunately, do nothing
}

// not for memory, some library code requires.
namespace std {
    void __throw_bad_function_call() {;}
}

#if 0
void mwx_static_init()
{
	for (void (**p)() = __init_array_start; p < __init_array_end; ++p)
		(*p)();
}
#endif
