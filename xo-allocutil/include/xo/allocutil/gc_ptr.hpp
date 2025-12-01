/** @file gc_ptr.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include <cstdint>

namespace xo {
    template <typename T>
    class gc_ptr;

    template <typename T>
    using gp = gc_ptr<T>;

    /** wrapper for a pointer to garbage-collector-eligible T.
     *  Application code will usually use the alias template gp<T>
     **/
    template <typename T>
    class gc_ptr {
    public:
        using element_type = T;

    public:
        gc_ptr() = default;
        gc_ptr(T * p) : ptr_{p} {}
        gc_ptr(const gc_ptr & x) : ptr_{x.ptr_} {}

        /** create from gc_ptr to some related type @tparam S **/
        template <typename S>
        gc_ptr(const gc_ptr<S> & x) : ptr_{x.ptr()} {}

        /** runtime downcast. shorthand for dynamic_cast<T*> **/
        template <typename S>
        static gc_ptr<T> from(const gc_ptr<S> & x) { return gc_ptr<T>{dynamic_cast<T*>(x.ptr())}; }

        /** convenience for static asserts **/
        static constexpr bool is_gc_ptr = true;
        /** see also: xo/refcnt/Refcounted.hpp **/
        static constexpr bool is_rc_ptr = false;

        static bool is_eq(gc_ptr x1, gc_ptr x2) {
            std::uintptr_t u1 = reinterpret_cast<std::uintptr_t>(x1.ptr());
            std::uintptr_t u2 = reinterpret_cast<std::uintptr_t>(x2.ptr());

            // multiple inheritance shenanigans.
            // (allow interface pointers separated by one pointer)

            if (u1 >= u2)
                return (u1 <= u2 + sizeof(std::uintptr_t));
            else
                return (u2 <= u1 + sizeof(std::uintptr_t));
        }

        /** (for consistency's sake) **/
        T * get() const { return ptr_; }

        T * ptr() const { return ptr_; }
        T ** ptr_address() { return &ptr_; }

        bool is_null() const { return ptr_ == nullptr; }
        void make_null() { ptr_ = nullptr; }

        void assign_ptr(T * x) { ptr_ = x; }

        gc_ptr & operator=(const gc_ptr & x) { ptr_ = x.ptr(); return *this; }

        T * operator->() const { return ptr_; }
        T & operator*() const { return *ptr_; }

    private:
        T * ptr_ = nullptr;
    };
} /*namespace xo*/

/* end gc_ptr.hpp */
