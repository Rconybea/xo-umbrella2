/** @file Pretty.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpSink.hpp"

namespace xo::print {
    /** @brief per-type opt-in to structured pretty-printing.
     *
     *  Specialize for a type @p T that should participate in pretty-printing,
     *  providing:
     *  @code
     *    static void print(PpSink & sink, const T & x);
     *  @endcode
     *
     *  The primary template is empty (no @c print), so @ref has_pretty is
     *  false for a type until @c Pretty<T> is specialized.  A type without a
     *  specialization renders via operator<< instead (see @ref pp_write).
     **/
    template <typename T>
    struct Pretty {};

    /** true iff @c Pretty<T> supplies @c print(PpSink&, const T&) **/
    template <typename T>
    concept has_pretty = requires (PpSink & sink, const T & x) {
        Pretty<T>::print(sink, x);
    };

    /** Emit @p x to @p sink.
     *
     *  Uses @c Pretty<T> when the type opts in (@ref has_pretty); otherwise
     *  falls back to operator<< via a streamed token.  A struct/container
     *  @c Pretty calls pp_write() on each member, so members whose type it
     *  doesn't control (operator<< only) render as atomic leaves.
     **/
    template <typename T>
    void pp_write(PpSink & sink, const T & x) {
        if constexpr (has_pretty<T>) {
            Pretty<T>::print(sink, x);
        } else {
            auto ins = sink.stream_open(1 /*min_z*/);
            ins << x;
            /* PpSinkInserter dtor commits the token */
        }
    }
} /*namespace xo::print*/

/* end Pretty.hpp */
