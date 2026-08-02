/** @file span_ppdetail.hpp
 *
 *  @author Roland Conybeare, Jul 2024
 *
 *  Legacy ostream printing for xo::mm::span (operator<<, printspan).
 *
 *  Split out from span.hpp so ppsink-focused subsystems can include span.hpp
 *  (the span type) without pulling in the operator<<-based printing machinery.
 *  Include this header only where you actually print a span to an ostream.
 *
 *  Structured (ostream-free) printing lives in span_pp.hpp, as
 *  Prettifier<span<CharT>>.  This header renders through the ppsink ostream
 *  bridges, so the two agree on escaping.
 **/

#pragma once

#include "xo/arena/span.hpp"
#include <xo/ppsink/quoted_ostream.hpp> /* os << quot(..) */
#include <xo/ppsink/tag_ostream.hpp>    /* os << xtag(..) */
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
               << xo::pp::xtag("addr", (void*)x.lo())
               << xo::pp::xtag("size", x.size())
               << " :text " << xo::pp::quot(std::string_view(x.lo(), x.hi()))
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

        /* (dropped: the legacy PPDETAIL_ATOMIC_BODY hooks for printspan_impl
         * and span.  ppsink customizes via Prettifier instead -- span has one
         * in span_pp.hpp, and printspan_impl needs none, since it renders its
         * raw content through the operator<< above.  The second hook named
         * xo::scm::span, which does not exist -- span lives in xo::mm -- and
         * only compiled because it was never instantiated.)
         */
    } /*namespace print*/
} /*namespace xo*/

/* end span_ppdetail.hpp */
