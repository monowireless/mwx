/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdint>

namespace mwx { inline namespace L1 {
    struct axis_xyzt {
        axis_xyzt(int i = 0) : x(i), y(i), z(i), t(i) {}
        axis_xyzt(int16_t x_, int16_t y_, int16_t z_, uint16_t t_) :
            x(x_), y(y_), z(z_), t(t_) {}
        axis_xyzt& operator = (int i) {
            x = i, y = i, z = i, t = i; return *this;
        }

        int16_t x;
        int16_t y;
        int16_t z;
        uint16_t t;

        int16_t& get_x() { return x; }
        int16_t& get_y() { return y; }
        int16_t& get_z() { return z; }
        uint16_t& get_t() { return t; }
    };

    // the iterator when storing container class.
    //   Iter             -> container class's iterator.
    //   T                -> element type of the container. (axis_xyzt)
    //   R& (T::*get)()   -> getting method.
    template <class Iter, typename T, typename R, R& (T::*get)()>
    class _iter_axis_xyzt {
        Iter _p;
    public:
        typedef int16_t value_type;
        typedef _iter_axis_xyzt self_type;
        typedef std::forward_iterator_tag iterator_category;
        typedef value_type* pointer;
        typedef value_type& reference;
        typedef int difference_type;

    public:
        _iter_axis_xyzt() {}
        _iter_axis_xyzt(Iter p) : _p(p) {}

        inline self_type& operator ++() {
            _p++;
            return *this;
        }

        inline self_type operator ++(int) {
            self_type p = *this;
            operator++();
            return p;
        }

        inline reference operator *() { return (*_p.*get)(); }
        // inline pointer operator->() { return ...; }
        inline bool operator == (self_type& itr2) { return !operator != (itr2); }
        inline bool operator == (self_type&& itr2) { return !operator != (itr2); }
        inline bool operator != (self_type& itr2) { return _p != itr2._p; }
        inline bool operator != (self_type&& itr2) { return _p != itr2._p; }
    };

    // iterator generator.
    // e.g.) auto&& px = get_axis_x_iter(cntobj.begin());
    template <class Iter, int16_t& (axis_xyzt::*get)()>
    using _iter_axis_xyzt_gen = _iter_axis_xyzt<Iter, axis_xyzt, int16_t, get>;

    template <class Iter>
    _iter_axis_xyzt_gen<Iter, &axis_xyzt::get_x> get_axis_x_iter(Iter p) {
        return _iter_axis_xyzt_gen<Iter, &axis_xyzt::get_x>(p);
    }
    
    template <class Iter>
    _iter_axis_xyzt_gen<Iter, &axis_xyzt::get_y> get_axis_y_iter(Iter p) {
        return _iter_axis_xyzt_gen<Iter, &axis_xyzt::get_y>(p);
    }
    
    template <class Iter>
    _iter_axis_xyzt_gen<Iter, &axis_xyzt::get_z> get_axis_z_iter(Iter p) {
        return _iter_axis_xyzt_gen<Iter, &axis_xyzt::get_z>(p);
    }

    #if 0
    template <class Iter>
    _iter_axis_xyzt<Iter, axis_xyzt, int16_t, &axis_xyzt::get_x> get_axis_x_iter(Iter p) {
        return _iter_axis_xyzt<Iter, axis_xyzt, int16_t, &axis_xyzt::get_x>(p);
    }
    template <class Iter>
    _iter_axis_xyzt<Iter, axis_xyzt, int16_t, &axis_xyzt::get_y> get_axis_y_iter(Iter p) {
        return _iter_axis_xyzt<Iter, axis_xyzt, int16_t, &axis_xyzt::get_y>(p);
    }
    template <class Iter>
    _iter_axis_xyzt<Iter, axis_xyzt, int16_t, &axis_xyzt::get_z> get_axis_z_iter(Iter p) {
        return _iter_axis_xyzt<Iter, axis_xyzt, int16_t, &axis_xyzt::get_z>(p);
    }
    #endif

    // class for just returing begin/end iterator.
    template <class Ixyz, class Cnt>
    class _axis_xyzt_iter_gen {
        Cnt& _c;
        
    public:
        _axis_xyzt_iter_gen(Cnt& c) : _c(c) {}
        Ixyz begin() { return Ixyz(_c.begin()); }
        Ixyz end() { return Ixyz(_c.end()); }
    };

    template <typename T, int16_t& (axis_xyzt::*get)()>
    using _axis_xyzt_axis_ret = _axis_xyzt_iter_gen<
            _iter_axis_xyzt<typename T::iterator, axis_xyzt, int16_t, get>, T>;
    
    template <typename T>
    _axis_xyzt_axis_ret<T, &axis_xyzt::get_x> get_axis_x(T& c) {
        return _axis_xyzt_axis_ret<T, &axis_xyzt::get_x>(c);
    }

    template <typename T>
    _axis_xyzt_axis_ret<T, &axis_xyzt::get_y> get_axis_y(T& c) {
        return _axis_xyzt_axis_ret<T, &axis_xyzt::get_y>(c);
    }

    template <typename T>
    _axis_xyzt_axis_ret<T, &axis_xyzt::get_z> get_axis_z(T& c) {
        return _axis_xyzt_axis_ret<T, &axis_xyzt::get_z>(c);
    }

#if 0
    template <typename T>
    _axis_xyzt_iter_gen<_iter_axis_xyzt<typename T::iterator, axis_xyzt, int16_t, &axis_xyzt::get_x>, T> get_axis_x(T& c) {
        return _axis_xyzt_iter_gen<_iter_axis_xyzt<typename T::iterator, axis_xyzt, int16_t, &axis_xyzt::get_x>, T>(c);
    }
    template <typename T>
    _axis_xyzt_iter_gen<_iter_axis_xyzt<typename T::iterator, axis_xyzt, int16_t, &axis_xyzt::get_y>, T> get_axis_y(T& c) {
        return _axis_xyzt_iter_gen<_iter_axis_xyzt<typename T::iterator, axis_xyzt, int16_t, &axis_xyzt::get_y>, T>(c);
    }
    template <typename T>
    _axis_xyzt_iter_gen<_iter_axis_xyzt<typename T::iterator, axis_xyzt, int16_t, &axis_xyzt::get_z>, T> get_axis_z(T& c) {
        return _axis_xyzt_iter_gen<_iter_axis_xyzt<typename T::iterator, axis_xyzt, int16_t, &axis_xyzt::get_z>, T>(c);
    }
#endif
}}