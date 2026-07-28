/** @file span_ppdetail.hpp
 *
 *  @author Roland Conybeare, Jul 2024
 *
 *  ostream printing for xo::mm::span (operator<<, printspan, legacy ppdetail).
 *
 *  Split out from span.hpp so ppsink-focused subsystems can include span.hpp
 *  (the span type) without pulling in the operator<<-based printing machinery.
 *  Include this header only where you actually print a span to an ostream.
 **/

#pragma once

#include "xo/arena/span.hpp"
#include "xo/indentlog/print/tag.hpp"
#include "xo/indentlog/print/ppdetail_atomic.hpp"
#include <ostream>
#include <string_view>

namespace xo {
    namespace mm {
        /** print a summary of @p x on stream @p os. Intended for diagnostics **/
        template <typename CharT>
        inline std::ostream &
        operator<<(std::ostream & os,
                   const span<CharT> & x) {
            os << "<span"
               << xtag("addr", (void*)x.lo())
               << xtag("size", x.size())
               << " :text " << xo::print::quot(std::string_view(x.lo(), x.hi()))
               << ">";
            return os;
        }
    } /*namespace mm*/

    namespace print {
        template <typename CharT>
        class printspan_impl {
        public:
            printspan_impl(xo::mm::span<CharT> x) : span_{x} {}

            xo::mm::span<CharT> span_;
        };

        template <typename CharT>
        printspan_impl<CharT> printspan(const xo::mm::span<CharT>& span) {
            return printspan_impl<CharT>(span);
        }

        template <typename CharT>
        inline std::ostream &
        operator<< (std::ostream & os,
                    const printspan_impl<CharT> & x)
        {
            for (const CharT * p = x.span_.lo(); p < x.span_.hi(); ++p)
                os << *p;

            return os;
        }

#ifndef ppdetail_atomic
        template <typename CharT>        \
        PPDETAIL_ATOMIC_BODY(printspan_impl<CharT>);

        template <typename CharT>        \
        PPDETAIL_ATOMIC_BODY(xo::scm::span<CharT>);
#endif

    } /*namespace print*/
} /*namespace xo*/

/* end span_ppdetail.hpp */
