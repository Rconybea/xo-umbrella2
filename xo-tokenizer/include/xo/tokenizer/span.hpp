/** @file span.hpp **/

#pragma once

#include "xo/indentlog/scope.hpp"
#include <ostream>
#include <cstdint>
#include <cassert>

namespace xo {
    namespace scm {
        /** @class span compression/span.hpp
         *
         *  @brief Represents a contiguous memory range,  without ownership.
         *
         *  @tparam CharT type for elements referred to by this span.
         **/
        template <typename CharT>
        class span {
        public:
            /** @brief typealias for span size (in units of CharT) **/
            using size_type = std::uint64_t;

        public:
            /** @brief create span for the contiguous memory range [@p lo, @p hi) **/
            span(CharT * lo, CharT * hi) : lo_{lo}, hi_{hi} {}

            /** @brief create a null span (i.e. with null @p lo, @p hi pointers) **/
            static span make_null() { return span(nullptr, nullptr); }

            /** @brief create span for C-style string @p cstr **/
            static span from_cstr(const CharT * cstr) {
                CharT * lo = cstr;
                CharT * hi = cstr ? cstr + strlen(cstr) : nullptr;

                return span(lo, hi);
            }

            /** @brief create span from std::string @p str **/
            static span from_string(const std::string& str) {
                CharT * lo = &(*str.begin());
                CharT * hi = &(*str.end());

                return span(lo, hi);
            }

            /** @brief concatenate two contiguous spans */
            static span concat(const span & span1, const span & span2) {
                if (span1.is_null())
                    return span2;
                if (span2.is_null())
                    return span1;

                if (span1.hi() != span2.lo()) {
                    scope log(XO_DEBUG(true));

                    log && log(xtag("span1.hi", (void*)span1.hi()), xtag("span2.lo", (void*)span2.lo()));
                }

                assert(span1.hi() == span2.lo());

                CharT * lo = span1.lo();
                CharT * hi = span2.hi();

                return span(lo, hi);
            }

            ///@{

            /** @name getters **/

            CharT * lo() const { return lo_; } /* get member span::lo_ */
            CharT * hi() const { return hi_; } /* get member span::hi_ */

            ///@}

            /** @brief create new span over supplied type,
             *  with identical (possibly misaligned) endpoints.
             *
             *  @warning
             *  1. New span uses exactly the same memory addresses.
             *     Endpoint pointers may not be aligned.
             *  2. Implementation assumes code compiled with
             *     @code -fno-strict-aliasing @endcode enabled.
             *
             *  @tparam OtherT element type for new span
             **/
            template <typename OtherT>
            span<OtherT>
            cast() const { return span<OtherT>(reinterpret_cast<OtherT *>(lo_),
                                               reinterpret_cast<OtherT *>(hi_)); }

            /** @brief create span including the first @p z members of this span. **/
            span prefix(size_type z) const { return span(lo_, lo_ + z); }

            /** @brief create span representing prefix up to (but not including) @p *p
             **/
            span prefix_upto(CharT * p) const {
                if (p <= hi_)
                    return span(lo_, p);
                else
                    return span(lo_, hi_);
            }

            /** @brief create span with first @p z members of this span removed **/
            span after_prefix(size_type z) const {
                if (lo_ + z > hi_)
                    z = hi_ - lo_;

                return span(lo_ + z, hi_);
            }

            /** @brief create span with @p prefix of this span removed **/
            span after_prefix(const span & prefix) const {
                assert(prefix.lo() == lo_);
                if (prefix.lo() != lo_) {
                    throw std::runtime_error
                        ("after_prefix: expected prefix of this span");
                }

                return after_prefix(prefix.size());
            }

            /** @brief create span starting with position p **/
            span suffix_from(CharT * p) const {
                if ((lo_ <= p) && (p <= hi_))
                    return span(p, hi_);
                else
                    return span(hi_, hi_);
            }

            /** @brief true iff this span is null.  distinct from empty. **/
            bool is_null() const { return lo_ == nullptr && hi_ == nullptr; }
            /** @brief true iff this span is empty (comprises 0 elements). **/
            bool empty() const { return lo_ == hi_; }
            /** @brief report the number of elements (of type CharT) in this span. **/
            size_type size() const { return hi_ - lo_; }

            span & operator+=(const span & x) {
                if (hi_ == x.lo_) {
                    hi_ = x.hi_;
                } else {
                    assert(false);
                }

                return *this;
            }

            /** print representation for this span on stream @p os **/
            void print(std::ostream & os) const {
                os << "<span"
                   << xtag("addr", (void*)lo_)
                   << xtag("size", size())
                   << " :text " << xo::print::quot(std::string_view(lo_, hi_))
                   << ">";
            }

        private:
            ///@{

            /** @brief start of span
                Span comprises memory address between @p lo (inclusive) and @p hi (exclusive)
            **/
            CharT * lo_ = nullptr;
            /** @brief end of span
                Span comprises memory address between @p lo (inclusive) and @p hi (exclusive)
            **/
            CharT * hi_ = nullptr;

            ///@}
        }; /*span*/

        template <typename CharT>
        inline bool
        operator==(const span<CharT> & lhs, const span<CharT> & rhs) {
            return ((lhs.lo() == rhs.lo())
                    && (lhs.hi() == rhs.hi()));
        }

        template <typename CharT>
        inline bool
        operator!=(const span<CharT> & lhs, const span<CharT> & rhs) {
            return ((lhs.lo() != rhs.lo())
                    || (lhs.hi() != rhs.hi()));
        }

        template <typename CharT>
        inline std::ostream &
        operator<<(std::ostream & os,
                   const span<CharT> & x) {
            x.print(os);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/
