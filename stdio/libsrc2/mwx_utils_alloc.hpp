 /* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
  * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
  * AGREEMENT).                                                   */
#pragma once

// defines three types allocator for array based class.


namespace mwx { inline namespace L1 {

static const int ALLOC_TYPE_LOCAL = 0;
static const int ALLOC_TYPE_HEAP = 1;
static const int ALLOC_TYPE_ATTACH = 2;
template <typename T>
class _mwx_alloc {
protected:
	T* _p;
	uint16_t _size;
public:
	_mwx_alloc() : _p(nullptr), _size(0) {}

};

template <typename T>
class alloc_heap : public _mwx_alloc<T> {
protected:
    T* _p_heap;
    uint16_t _sz;
public:
    using super = _mwx_alloc<T>;
	alloc_heap() : _p_heap(nullptr), _sz(0) {}
	// ~alloc_heap() { if (super::_p) delete(super::_p); } // no delete on the system!

	void init_heap(uint16_t s) {
        if (_p_heap == nullptr) {
            _sz = s;
            _p_heap = new T[s];

            super::_p = _p_heap;
            super::_size = _sz;
        }
	}

    void re_attach() {
        super::_p = _p_heap;
        super::_size = _sz;
    }

    uint16_t alloc_size() {
        return _sz;
    }
    
    void _is_heap() {}
    static const int _type = ALLOC_TYPE_HEAP;
};

template <typename T, int N>
class alloc_local : public _mwx_alloc<T> {
protected:
	T _a[N];
public:
	using super = _mwx_alloc<T>;
	alloc_local() {
        init_local();
	}

    void init_local() {
		super::_p = _a;
		super::_size = N;
    }

    void re_attach() {
        init_local();
    }

    uint16_t alloc_size() {
        return N;
    }

    void _is_local() {}
    static const int _type = ALLOC_TYPE_LOCAL;
};

template <typename T>
class alloc_attach : public _mwx_alloc<T> {
protected:
public:
    using super = _mwx_alloc<T>;
	alloc_attach() {}
	
	void attach(T* p, int n) {
		super::_p = p;
		super::_size = n;
	}
    
    void re_attach() {
        ; // do nothing
    }

    uint16_t alloc_size() {
        return super::_size;
    }

    void _is_attach() {}
    static const int _type = ALLOC_TYPE_ATTACH;
};

}}