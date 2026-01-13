/** @file DString.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/obj.hpp>
#include <cstdint>

namespace xo {
    namespace scm {
        /** @class DString
         *  @brief String implementation with gc hooks
         *
         *  String implementation for Schematika.
         *  Size-prefixed and null-terminated.
         *  Note however that string length != size for utf-8.
         *
         *  Uses flexible array for chars,
         *  with string contents in memory immediately
         *  following the DString itself
         **/
        struct DString {
        public:
            using size_type = std::uint32_t;
            using AAllocator = xo::mm::AAllocator;

            /** create empty string with space for @cap chars
             *  (including null terminator).
             *  Use memory from allocator @p mm
             **/
            static DString * empty(obj<AAllocator> mm,
                                   size_type cap);

            /** create string containing a copy of null-terminated @p cstr.
             *  Use memory from allocator @p mm
             **/
            static DString * from_cstr(obj<AAllocator> mm,
                                       const char * cstr);

            size_type capacity() const noexcept { return capacity_; }
            size_type size() const noexcept { return size_; }
            const char * chars() const noexcept { return chars_; }

        private:
            /** extent of @ref chars_ array **/
            size_type capacity_ = 0;
            /** null terminator at @c chars_[size_] **/
            size_type size_ = 0;
            /** string contents **/
            char chars_[];
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end DString.hpp */
