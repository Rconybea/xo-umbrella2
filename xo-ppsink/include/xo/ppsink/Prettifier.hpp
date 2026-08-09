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
#include <charconv>
#include <string_view>

namespace xo::pp {
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

    /** default precision for floating-point leaves.
     *
     *  6 significant digits, matching std::ostream's default -- which is what
     *  a double rendered as BEFORE this specialization existed, by falling
     *  through the empty primary template to operator<<.  Preserved
     *  deliberately: changing it changes every float rendering in xo, and that
     *  is a decision to take on its own, not a side effect of moving the
     *  default somewhere visible.
     *
     *  See .xo-backlog/xo-ppsink/issues/07-nested-formatting-context.md
     **/
    static constexpr int c_default_float_precision = 6;

    /** scalar leaf: render a double via std::to_chars (ostream-free).
     *
     *  chars_format::general with precision 6 is exactly printf's "%.6g", and
     *  so exactly what std::ostream produced by default.  Note this is NOT
     *  to_chars' no-precision overload, which gives the shortest round-trip
     *  form (1.0/3.0 -> 0.3333333333333333) and would change every rendering.
     *
     *  THIS IS THE ENTRY POINT for future formatting control: a nested
     *  formatting context can be reached through @p sink without changing this
     *  signature, since the sink is already in hand.  See issue 07.
     *
     *  Stream state no longer applies, and that is the point rather than a
     *  concession.  Previously a caller who set precision on the ostream under
     *  a FlatSink would see it honoured, because rendering went through
     *  operator<<.  Formatting now belongs to the sink, not to whatever stream
     *  happens to be beneath it.
     *
     *  Legacy already treated stream state as something to work around rather
     *  than use: xo-indentlog/print/fixed.hpp saves flags() and precision(),
     *  sets its own, prints, and restores them -- six lines of ceremony purely
     *  to stop one value's formatting leaking into a shared stream.  Nothing
     *  in xo relied on ambient stream state; it defended against it.
     **/
    template <>
    struct Prettifier<double> {
        static void print(PpSink & sink, double x) {
            /* "%.6g" is at most ~13 chars ("-1.23457e-308"); buf[32] is ample */
            char buf[32];

            auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), x,
                                           std::chars_format::general,
                                           c_default_float_precision);

            if (ec != std::errc()) {
                /* cannot happen with buf[32] at precision 6, but do not emit
                 * garbage if it ever does
                 */
                sink.put("<double?>");
                return;
            }

            sink.put(std::string_view(buf,
                                      static_cast<std::size_t>(ptr - buf)));
        }
    };

    /** scalar leaf: render a float exactly as a double.
     *
     *  Deliberately not narrower: a float promoted and printed at 6
     *  significant digits is what operator<< did, so this preserves it.
     **/
    template <>
    struct Prettifier<float> {
        static void print(PpSink & sink, float x) {
            Prettifier<double>::print(sink, static_cast<double>(x));
        }
    };
} /*namespace xo::pp*/

/* end Prettifier.hpp */
