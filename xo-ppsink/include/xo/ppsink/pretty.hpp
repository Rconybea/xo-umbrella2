/** @file pretty.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  pretty(sink, x): the dispatch verb for structured printing.
 *
 *  Kept separate from Prettifier.hpp (the customization *class*) so the two
 *  are easy to tell apart: specialize @c Prettifier<T> to teach a type how to
 *  print; include this header and call @c pretty() to actually print one.
 *
 *  Only the operator<< fallback needs <ostream>, and only at its point of
 *  instantiation -- so this header itself does not include <ostream>.
 **/

#pragma once

#include "Prettifier.hpp"
#include <string_view>
#include <type_traits>

namespace xo::pp {
    /** Emit @p x to @p sink.
     *
     *  Dispatch, in order:
     *  1. @c Prettifier<T> when the type opts in (@ref has_prettifier) -- the
     *     native path: writes directly to the PpSink and can emit begin/split/end.
     *  2. string-like leaf (convertible to std::string_view) -- straight to
     *     PpSink::put, no ostream.
     *  3. fallback: operator<< via a streamed token.  This is the ONLY path
     *     that needs <ostream> (at the point of instantiation); prefer a
     *     Prettifier<T> specialization so a type never lands here.
     **/
    template <typename T>
    void pretty(PpSink & sink, const T & x) {
        if constexpr (has_prettifier<T>) {
            Prettifier<T>::print(sink, x);
        } else if constexpr (std::is_convertible_v<T, std::string_view>) {
            sink.put(std::string_view(x));
        } else {
            auto ins = sink.stream_open(1 /*min_z*/);
            ins << x;
            /* PpSinkInserter dtor commits the token */
        }
    }

    /** member-convenience spelling of pretty(*this, x); see PpSink::pp **/
    template <typename T>
    void
    PpSink::pp(const T & x) {
        pretty(*this, x);
    }
} /*namespace xo::pp*/

/* end pretty.hpp */
