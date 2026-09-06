/** @file hex.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  hex_view(range, style): print a range of bytes in hexadecimal.  The
 *  ppsink-native replacement for legacy xo-indentlog xo::hex_view
 *  (xo/indentlog/print/hex.hpp).
 *
 *  @code
 *    sink.pp(hex_view(s));                          // [68 65 6c 6c 6f]
 *    sink.pp(hex_view(s, hexstyle::with_char));     // [68(h) 65(e) 6c(l) ..]
 *  @endcode
 *
 *  Printable-character detection uses an explicit ASCII range test rather
 *  than std::isprint, so output does not vary with the active locale.  The
 *  two agree in the "C" locale.
 *
 *  This header is ostream-free; see hex_ostream.hpp to stream a hex_view
 *  directly to an ostream.  (Using one as a tag value -- xtag("k", hex_view(s))
 *  -- needs only this header, since the tag machinery renders it through the
 *  sink.)
 **/

#pragma once

#include "Prettifier.hpp"
#include <ranges>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace xo::pp {
    /** @brief whether to annotate each byte with its ascii character **/
    enum class hexstyle {
        /** just the hex digits:  @c [68 65 6c] **/
        bare,
        /** hex digits plus the character:  @c [68(h) 65(e) 6c(l)] **/
        with_char,
    };

    /** @brief whether to qualify a byte with the @c 0x radix prefix. **/
    enum class hexprefix {
        /** just the digits:  @c fd **/
        plain,
        /** 0x-qualified:  @c 0xfd **/
        qualified,
    };

    namespace detail {
        /** @brief a contiguous range of 1-byte elements.
         *
         *  Structural, not nominal: xo::mm::span and xo::scm::span satisfy it
         *  without ppsink naming (or depending on) xo-arena or xo-tokenizer.
         **/
        template <typename R>
        concept byte_range = (std::ranges::contiguous_range<R>
                              && std::ranges::sized_range<R>
                              && sizeof(std::ranges::range_value_t<R>) == 1);
    } /*namespace detail*/

    /** @brief a range (unowned) of bytes, to print in hexadecimal.
     *
     *  Refers to memory it does not own, so the range must outlive the
     *  hex_view.  Safe in a single print expression; bind to a named local
     *  first if you need it to live longer.
     **/
    class hex_view {
    public:
        /** number of bytes per output row.  Rows are the only break
         *  opportunities, and match the row width a hexdump-style renderer
         *  would use.
         **/
        static constexpr std::size_t c_bytes_per_row = 16;

    public:
        /** print the byte range [@p lo, @p hi) **/
        constexpr hex_view(const std::uint8_t * lo,
                           const std::uint8_t * hi,
                           hexstyle style = hexstyle::bare) noexcept
            : lo_{lo}, hi_{hi}, style_{style} {}

        /* the ctors below are deliberately not constexpr: reinterpret_cast is
         * not allowed in a constant expression, so marking them constexpr
         * would be a promise the compiler can never honour (-Winvalid-constexpr).
         */

        /** print the char range [@p lo, @p hi) **/
        hex_view(const char * lo,
                 const char * hi,
                 hexstyle style = hexstyle::bare) noexcept
            : lo_{reinterpret_cast<const std::uint8_t *>(lo)},
              hi_{reinterpret_cast<const std::uint8_t *>(hi)},
              style_{style} {}

        /** print the bytes of @p r.
         *
         *  explicit: an implicit conversion from every byte range would make
         *  hex_view a candidate in unrelated overload sets.
         **/
        template <detail::byte_range R>
        explicit hex_view(const R & r,
                          hexstyle style = hexstyle::bare)
            : lo_{reinterpret_cast<const std::uint8_t *>(std::ranges::data(r))},
              hi_{lo_ + std::ranges::size(r)},
              style_{style} {}

        constexpr const std::uint8_t * lo() const noexcept { return lo_; }
        constexpr const std::uint8_t * hi() const noexcept { return hi_; }
        constexpr hexstyle style() const noexcept { return style_; }
        constexpr std::size_t size() const noexcept {
            return static_cast<std::size_t>(hi_ - lo_);
        }

    private:
        /** print bytes starting here **/
        const std::uint8_t * lo_;
        /** print bytes up to (not including) here **/
        const std::uint8_t * hi_;
        /** whether to annotate each byte with its character **/
        hexstyle style_;
    };

    /** @brief a single byte, to print in hexadecimal.
     *
     *  @code
     *    sink.pp(hex(0xfd));                          // fd
     *    sink.pp(hex(0x4f, hexstyle::with_char));     // 4f(O)
     *    sink.pp(hex(0xfd, hexprefix::qualified));    // 0xfd
     *  @endcode
     *
     *  Defaults to unprefixed, matching hex_view (which renders [68 65 6c],
     *  not [0x68 ..]) and legacy xo::hex.  Pass hexprefix::qualified where
     *  the surrounding context does not already say "this is hex" -- a lone
     *  field among decimal ones, say.
     *
     *  Unlike hex_view this owns its byte, so it has no lifetime constraint.
     **/
    class hex {
    public:
        /** print @p x in hexadecimal, annotated per @p style,
         *  qualified per @p prefix
         **/
        constexpr explicit hex(std::uint8_t x,
                               hexstyle style = hexstyle::bare,
                               hexprefix prefix = hexprefix::plain) noexcept
            : x_{x}, style_{style}, prefix_{prefix} {}

        /** ctor with prefix-argument first **/
        constexpr explicit hex(std::uint8_t x,
                               hexprefix prefix,
                               hexstyle style = hexstyle::bare) noexcept
            : x_{x}, style_{style}, prefix_{prefix} {}

        constexpr std::uint8_t value() const noexcept { return x_; }
        constexpr hexstyle style() const noexcept { return style_; }
        constexpr hexprefix prefix() const noexcept { return prefix_; }

    private:
        /** the byte to print **/
        std::uint8_t x_;
        /** whether to annotate with the ascii character **/
        hexstyle style_;
        /** whether to qualify with the 0x radix prefix **/
        hexprefix prefix_;
    };

    namespace detail {
        /** lowercase nibble -> hex digit **/
        inline constexpr char c_hex_digit[] = "0123456789abcdef";

        /** true iff @p uc is a printable ascii character.
         *  Deliberately not std::isprint: locale-independent, so test
         *  expectations do not shift with the environment.
         **/
        inline constexpr bool
        is_ascii_printable(std::uint8_t uc) noexcept {
            return (uc >= 0x20) && (uc < 0x7f);
        }

        /** emit one byte as a single token: @c 6c , or @c 6c(l) with @p style,
         *  or @c 0x6c(l) with @p prefix
         **/
        inline void
        put_hex_byte(PpSink & sink,
                     std::uint8_t uc,
                     hexstyle style,
                     hexprefix prefix = hexprefix::plain) {
            /* "0xff(c)" is the widest form */
            char buf[7];
            std::size_t n = 0;

            if (prefix == hexprefix::qualified) {
                buf[n++] = '0';
                buf[n++] = 'x';
            }

            buf[n++] = c_hex_digit[(uc >> 4) & 0x0f];
            buf[n++] = c_hex_digit[uc & 0x0f];

            if (style == hexstyle::with_char) {
                buf[n++] = '(';
                buf[n++] = (is_ascii_printable(uc)
                            ? static_cast<char>(uc)
                            : '?');
                buf[n++] = ')';
            }

            /* one put() per byte: a byte must never be split across lines */
            sink.put(std::string_view(buf, n));
        }
    } /*namespace detail*/

    /** @brief render a hex into a PpSink.
     *
     *  One put(), no group: a byte is a single token and must never be split
     *  across lines.  Shares detail::put_hex_byte() with Prettifier<hex_view>,
     *  so a byte renders identically alone or inside a dump.
     **/
    template <>
    struct Prettifier<hex> {
        static void print(PpSink & sink, const hex & x) {
            detail::put_hex_byte(sink, x.value(), x.style(), x.prefix());
        }
    };

    /** @brief render a hex_view into a PpSink.
     *
     *  Shape notes -- these differ from pretty_struct.hpp on purpose:
     *
     *  - put("[") *before* begin(), so the bracket stays on the opening line.
     *    Same rule as pretty_struct's put(name).
     *  - NO leading split after begin().  pretty_struct emits a split before
     *    every field including the first; here legacy renders "[68 .." with no
     *    space after the bracket, so a leading split would add one.  Please
     *    don't "fix" this to match pretty_struct.
     *  - split(1) only at row boundaries; plain put(" ") between bytes within
     *    a row.  Splits are break opportunities, so a split between every byte
     *    would let a line break mid-row.
     *  - put("]") *before* end(), so the closing bracket counts toward the
     *    group's width when the sink decides whether the group fits.
     **/
    template <>
    struct Prettifier<hex_view> {
        static void print(PpSink & sink, const hex_view & x) {
            sink.put("[");
            sink.begin();

            std::size_t i = 0;
            for (const std::uint8_t * p = x.lo(); p < x.hi(); ++p, ++i) {
                if (i > 0) {
                    if (i % hex_view::c_bytes_per_row == 0)
                        sink.split(1);
                    else
                        sink.put(" ");
                }

                detail::put_hex_byte(sink, *p, x.style());
            }

            sink.put("]");
            sink.end();
        }
    };
} /*namespace xo::pp*/

/* end hex.hpp */
