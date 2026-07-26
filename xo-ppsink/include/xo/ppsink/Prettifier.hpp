/** @file Prettifier.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Prettifier<T>: the per-type customization *class* for structured printing.
 *  Specialize it to teach a type how to render into a PpSink.  The dispatch
 *  verb that consults it, pretty(sink, x), lives in pretty.hpp.
 **/

#pragma once

#include "PpSink.hpp"
#include <string_view>
#include <charconv>

namespace xo::print {
    /** @brief per-type opt-in to structured pretty-printing.
     *
     *  Specialize for a type @p T that should participate in pretty-printing,
     *  providing:
     *  @code
     *    static void print(PpSink & sink, const T & x);
     *  @endcode
     *
     *  The primary template is empty (no @c print), so @ref has_prettifier is
     *  false for a type until @c Prettifier<T> is specialized.  A type without a
     *  specialization renders as a leaf (see @ref pretty): string-like via
     *  PpSink::put, anything else via operator<<.
     **/
    template <typename T>
    struct Prettifier {};

    /** true iff @c Prettifier<T> supplies @c print(PpSink&, const T&) **/
    template <typename T>
    concept has_prettifier = requires (PpSink & sink, const T & x) {
        Prettifier<T>::print(sink, x);
    };

    /** scalar leaf: render an int via std::to_chars (ostream-free).
     *
     *  A signed int is at most 11 chars ("-2147483648"); buf[16] is ample.
     **/
    template <>
    struct Prettifier<int> {
        static void print(PpSink & sink, int x) {
            char buf[16];
            auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), x);
            (void)ec; /* cannot fail: buf is large enough for any int */
            sink.put(std::string_view(buf,
                                      static_cast<std::size_t>(ptr - buf)));
        }
    };
} /*namespace xo::print*/

/* end Prettifier.hpp */
