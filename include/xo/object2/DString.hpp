/** @file DString.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/obj.hpp>
#include <string_view>
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

            /** @defgroup dstring-ctors constructors **/
            ///@{

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

            ///@}
            /** @defgroup dstring-access access methods **/
            ///@{

            size_type capacity() const noexcept { return capacity_; }
            size_type size() const noexcept { return size_; }
            const char * chars() const noexcept { return chars_; }

            ///@}
            /** @defgroup dstring-iterators iterators **/
            ///@{

            ///@}
            /** @defgroup dstring-assign assignment **/
            ///@{

            /** put string into empty state **/
            void clear() noexcept { size_ = 0; chars_[0] = '\0'; }

            /** replace contents with @p other, or prefix of up to @p capacity - 1 chars **/
            DString & assign(const DString & other);

            // TODO - behave like std::string, to the extent feasible
            //   insert
            //   insert_range
            //   erase
            //   push_back
            //   append
            //   append_range
            //   operator+=
            // replace
            // replace_with_range
            // copy
            // find
            // rfind
            // find_first_of
            // find_first_not_of
            // find_last_of
            // find_last_not_of
            // compare
            // starts_with
            // end_with
            // contains
            // substr

            ///@}
            /** @defgroup dstring-conversion-operators conversion operators **/
            ///@{

            operator std::string_view() const noexcept { return std::string_view(chars_); }

            /** @brief conversion oeprator to C-style string.
             *
             *  Example
             *  @code
             *    DString s = ...;
             *    ::strcmp(s, "obey...");
             *  @endcode
             **/
            operator const char * () const noexcept { return &(chars_[0]); }

            ///@}

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
