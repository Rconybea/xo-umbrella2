/** @file DString.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/alloc2/Allocator.hpp>
#include <xo/gc/Collector.hpp>
#include <xo/facet/obj.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <string_view>
#include <cstdint>
#include <cstdio>

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
            /** @defgroup dstring-types type traits **/
            ///@{
            /** character traits for this DString **/
            using traits_type = std::char_traits<char>;
            /** type of each character in this DString **/
            using value_type = char;
            using size_type = std::uint32_t;
            /** representation for a read/write iterator **/
            using iterator = char *;
            /** representation for a readonly iterator **/
            using const_iterator = const char *;
            /** xo allocator **/
            using AAllocator = xo::mm::AAllocator;
            /** garbage collector **/
            using ACollector = xo::mm::ACollector;
            /** ppindentinfo for APrintable **/
            using ppindentinfo = xo::print::ppindentinfo;
            ///@}
            /** @defgroup dstring-ctors constructors **/
            ///@{

            /** default ctor **/
            DString() = default;

            /** not simply copyable, because of flexible array.
             *  Need allocator
             **/
            DString(const DString &) = delete;

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

            /** clone existing string **/
            static DString * clone(obj<AAllocator> mm,
                                   const DString * src);

#ifdef NOT_YET
            /** **/
            static DString * concat(obj<AAllocator> mm,
                                    DString * s1,
                                    DString * s2);
#endif

            ///@}
            /** @defgroup dstring-access access methods **/
            ///@{

            /** get writeable access to string representation.
             *  Caller responsible for calling fixup() if string length modified
             **/
            char * data() noexcept { return chars_; }

            /** return char at position @p pos in this string, counting from zero.
             *  Does not check bounds. Undefined behavior if @p pos = @ref capacity_
             **/
            char & operator[](size_type pos) noexcept { return chars_[pos]; }
            const char & operator[](size_type pos) const noexcept { return chars_[pos]; }

            size_type capacity() const noexcept { return capacity_; }
            size_type size() const noexcept { return size_; }
            const char * chars() const noexcept { return chars_; }

            ///@}
            /** @defgroup dstring-iterators iterators **/
            ///@{
            iterator begin() noexcept { return &chars_[0]; }
            iterator end() noexcept { return &chars_[size_]; }

            const_iterator cbegin() const noexcept { return &chars_[0]; }
            const_iterator cend() const noexcept { return &chars_[size_]; }
            const_iterator begin() const noexcept { return cbegin(); }
            const_iterator end() const noexcept { return cend(); }

            ///@}
            /** @defgroup dstring-assign assignment **/
            ///@{

            /** put string into empty state **/
            void clear() noexcept { size_ = 0; chars_[0] = '\0'; }

            /** replace contents with @p other, or prefix of up to @p capacity - 1 chars **/
            DString & assign(const DString & other);
            ///@}
            /** @defgroup dstring-general general methods **/
            ///@{

            /** format string into this DString using printf-style formatting.
             *  Truncates if result exceeds capacity.
             *  @return number of characters written (excluding null terminator)
             **/
            template <typename... Args>
            size_type sprintf(const char * fmt, Args&&... args) {
                int n = std::snprintf(chars_, capacity_, fmt, std::forward<Args>(args)...);
                if (n < 0) {
                    size_ = 0;
                    chars_[0] = '\0';
                } else {
                    size_ = (n < static_cast<int>(capacity_)) ? n : capacity_ - 1;
                }
                return size_;
            }

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

            /** recalculate string size if string contents modified without
             *  through side effects
             **/
            size_type fixup_size() noexcept;

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
            /** @defgroup dstring-printable-methods printable facet methods **/
            ///@{

            bool pretty(const ppindentinfo & ppii) const;

            ///@}
            /** @defgroup dstring-gcobject-methods gcobject facet methods **/
            ///@{

            size_type shallow_size() const noexcept;

            /** clone string, using memory from allocator @p mm **/
            DString * shallow_copy(obj<AAllocator> mm) const noexcept;

            /** fixup child pointers (trivial for DString, no children) **/
            size_type forward_children(obj<ACollector> gc) noexcept;

            ///@}

        private:
            /** @defgroup dstring-instance-variables instance variables **/
            ///@{

            /** extent of @ref chars_ array **/
            size_type capacity_ = 0;
            /** null terminator at @c chars_[size_] **/
            size_type size_ = 0;
            /** string contents **/
            char chars_[];

            ///@}
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end DString.hpp */
