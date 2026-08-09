/** @file color_spec.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  A color VALUE (color_spec_type) and the process-wide gate, with no
 *  dependency on PpSink beyond a forward declaration.
 *
 *  Split out of color.hpp so that PpStyle.hpp -- which holds color values by
 *  value, and which PpSink.hpp includes -- does not have to include a header
 *  that needs PpSink complete.  color.hpp still supplies everything it used
 *  to; nothing that included it needs to change.
 *
 *  Emission (color_guard, with_color) stays in color.hpp: those call
 *  sink.put(), so they need PpSink complete and cannot live here.
 **/

#pragma once

#include <cstdint>

namespace xo::pp {
    class PpSink;

    /** how a color code is encoded (ANSI SGR subset) **/
    enum class color_encoding : std::uint8_t {
        none,   /*!< no color */
        ansi,   /*!< 4-bit: 30..37, 90..97          -> \033[<code>m       */
        xterm,  /*!< 8-bit: 0..255                   -> \033[38;5;<code>m  */
        rgb,    /*!< 24-bit: r<<16 | g<<8 | b        -> \033[38;2;r;g;bm   */
    };

    /** @brief a foreground color specification (ANSI SGR) **/
    class color_spec_type {
    public:
        color_spec_type() = default;
        color_spec_type(color_encoding encoding, std::uint32_t code)
            : encoding_{encoding}, code_{code} {}

        static color_spec_type none()  { return color_spec_type(); }
        static color_spec_type ansi(std::uint32_t code)  { return {color_encoding::ansi,  code}; }
        static color_spec_type xterm(std::uint32_t code) { return {color_encoding::xterm, code}; }
        static color_spec_type rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
            return {color_encoding::rgb,
                    (std::uint32_t(r) << 16) | (std::uint32_t(g) << 8) | std::uint32_t(b)};
        }

        /* 4-bit named foreground colors */
        static color_spec_type black()   { return ansi(30); }
        static color_spec_type red()     { return ansi(31); }
        static color_spec_type green()   { return ansi(32); }
        static color_spec_type yellow()  { return ansi(33); }
        static color_spec_type blue()    { return ansi(34); }
        static color_spec_type magenta() { return ansi(35); }
        static color_spec_type cyan()    { return ansi(36); }
        static color_spec_type white()   { return ansi(37); }

        color_encoding encoding() const { return encoding_; }
        std::uint32_t code() const { return code_; }

        /** emit this color's SGR "on" escape to @p sink as one token
         *  (no-op when encoding is none)
         **/
        void fg_on(PpSink & sink) const;
        /** emit the SGR reset escape to @p sink (no-op when encoding is none) **/
        void fg_off(PpSink & sink) const;

    private:
        color_encoding encoding_ = color_encoding::none;
        std::uint32_t code_ = 0;
    };
} /*namespace xo::pp*/

/* end color_spec.hpp */
