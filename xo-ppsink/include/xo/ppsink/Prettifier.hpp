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
#include <cstdint>
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

    /** Integer types that should render as NUMBERS.
     *  Takes all integral types except for bool and char-oriented types
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

    /** Use:
     *    PpSink sink = ...;
     *    using T = uint32_t; // or another integral type
     *    T x = 123;
     *    Prettifier<T>::print(sink, x);
     *  writes
     *    "123" to sink
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

    /** Use:
     *    PpSink sink = ...;
     *    Prettifier<double>::print(sink, true)
     *  writes
     *    "1" to sink.
     **/
    template <>
    struct Prettifier<bool> {
        static void print(PpSink & sink, bool x) {
            sink.put(x ? std::string_view("1") : std::string_view("0"));
        }
    };

    /** default precision for floating-point leaves.
     *  Matching ostream's default as placeholder for something more elaborate.
     **/
    static constexpr int c_default_float_precision = 6;

    /** Use:
     *    PpSink sink = ...;
     *    Prettifier<double>::print(sink, 0.1234)
     *  writes
     *    "0.1234" to sink.
     *
     *  Equivalent to "%.6g" for now
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

    /** promote float -> double for printing.
     *  Consistent with operator<< policy
     **/
    template <>
    struct Prettifier<float> {
        static void print(PpSink & sink, float x) {
            Prettifier<double>::print(sink, static_cast<double>(x));
        }
    };

    namespace detail {
        /** Use:
         *    PpSink sink = ...;
         *    const void *x = 0x1234;
         *    detail::pp_put_pointer(sink, x);
         * writes
         *    "0x1234" to sink
         *
         * nullptr formatted as plain
         *    "0"
         **/
        inline void pp_put_pointer(PpSink & sink, const void * p) {
            if (!p) {
                sink.put(std::string_view("0"));
                return;
            }

            /* "0x" + two hex digits per byte; +1 keeps to_chars' end in range */
            char buf[2 + (2 * sizeof(void *)) + 1];
            buf[0] = '0';
            buf[1] = 'x';

            auto [ptr, ec] = std::to_chars(buf + 2, buf + sizeof(buf),
                                           reinterpret_cast<std::uintptr_t>(p),
                                           16);
            (void)ec; /* cannot fail: buf holds every pointer width */

            sink.put(std::string_view(buf,
                                      static_cast<std::size_t>(ptr - buf)));
        }
    }

    template <>
    struct Prettifier<void *> {
        static void print(PpSink & sink, const void * x) {
            detail::pp_put_pointer(sink, x);
        }
    };
    template <>
    struct Prettifier<const void *> {
        static void print(PpSink & sink, const void * x) {
            detail::pp_put_pointer(sink, x);
        }
    };
} /*namespace xo::pp*/

/* end Prettifier.hpp */
