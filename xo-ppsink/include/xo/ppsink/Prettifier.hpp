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
#include <concepts>
#include <string_view>
#include <type_traits>

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

    /** integer types that should render as NUMBERS.
     *
     *  Deliberately not simply std::integral.  `char`, `signed char`,
     *  `unsigned char` and the char8/16/32/wchar types are integral, but
     *  operator<< renders them as CHARACTERS -- measured 2026-08-10:
     *  `'A'`, `(signed char)65` and `(unsigned char)65` all print "A", and
     *  `(std::int8_t)-1` prints one garbage byte.  Bringing them under a
     *  to_chars Prettifier would turn 'A' into "65" everywhere in xo, which is
     *  an output-visible change and not the point of this specialization.
     *
     *  NB this makes std::int8_t / std::uint8_t character types on any normal
     *  platform, since they are typedefs for (un)signed char.  That is
     *  inherited from operator<<, not chosen here.
     *
     *  `bool` is excluded too, and handled separately below.
     **/
    template <typename T>
    concept pp_number_integral
        = std::integral<T>
       && !std::same_as<std::remove_cv_t<T>, bool>
       && !std::same_as<std::remove_cv_t<T>, char>
       && !std::same_as<std::remove_cv_t<T>, signed char>
       && !std::same_as<std::remove_cv_t<T>, unsigned char>
       && !std::same_as<std::remove_cv_t<T>, wchar_t>
       && !std::same_as<std::remove_cv_t<T>, char8_t>
       && !std::same_as<std::remove_cv_t<T>, char16_t>
       && !std::same_as<std::remove_cv_t<T>, char32_t>;

    /** scalar leaf: render an integer via std::to_chars (ostream-free).
     *
     *  Covers every width -- short, int, long, long long and their unsigned
     *  forms, hence also std::size_t, std::uint32_t and friends.  Before this
     *  existed only `int` had a Prettifier, so `std::uint32_t` (e.g. a
     *  container's size()) fell through to the operator<< fallback: correct
     *  output, but via an ostream, and invisibly.
     *
     *  Output-neutral by construction: to_chars and operator<< agree on
     *  decimal integers, verified across all widths in
     *  xo-ppsink/utest/Prettifier.test.cpp.
     *
     *  An unsigned 64-bit value is at most 20 digits, a signed one 19 plus a
     *  sign; buf[24] is ample for every width.
     **/
    template <pp_number_integral T>
    struct Prettifier<T> {
        static void print(PpSink & sink, T x) {
            char buf[24];
            auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), x);
            (void)ec; /* cannot fail: buf is large enough for any integer */
            sink.put(std::string_view(buf,
                                      static_cast<std::size_t>(ptr - buf)));
        }
    };

    /** scalar leaf: render a bool as "1"/"0", ostream-free.
     *
     *  **Preserves the current rendering, deliberately.**  operator<< prints a
     *  bool as 1/0 unless std::boolalpha is set, and nothing in xo sets it --
     *  measured 2026-08-10.  Renderings already pinned across the tree contain
     *  it (e.g. TypeDescr's ":complete 1",
     *  xo-expression2/utest/printable_render.test.cpp).
     *
     *  So this specialization removes an ostream round-trip and changes no
     *  output.  Whether "true"/"false" would be BETTER is open and is now a
     *  one-line change in one place rather than a property inherited from
     *  <ostream> -- the same framing as c_default_float_precision above.  It
     *  would be output-visible and wants its own commit.
     **/
    template <>
    struct Prettifier<bool> {
        static void print(PpSink & sink, bool x) {
            sink.put(x ? std::string_view("1") : std::string_view("0"));
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
