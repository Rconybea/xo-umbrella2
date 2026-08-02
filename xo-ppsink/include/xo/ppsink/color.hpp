/** @file color.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  ANSI foreground color for the ppsink pretty-printer.
 *
 *  Color escapes are non-printing: PpState::count_visible_chars skips a
 *  \033[...m sequence, so colored text does not affect margin/fit accounting.
 *  (Each escape must be emitted as a SINGLE put() token -- the skip state is
 *  per-token -- which color_spec_type::fg_on/fg_off guarantee.)
 *
 *  Global gate: color_config::color_enabled (default false -> no escapes).
 **/

#pragma once

#include "PpSink.hpp"
#include <cstdint>
#include <string_view>

namespace xo::pp {
    /** how a color code is encoded (ANSI SGR subset) **/
    enum class color_encoding : std::uint8_t {
        none,   /*!< no color */
        ansi,   /*!< 4-bit: 30..37, 90..97          -> \033[<code>m       */
        xterm,  /*!< 8-bit: 0..255                   -> \033[38;5;<code>m  */
        rgb,    /*!< 24-bit: r<<16 | g<<8 | b        -> \033[38;2;r;g;bm   */
    };

    /** process-wide color gate.  When false, color_guard/with_color emit no
     *  escapes (contents print uncolored), so plain runs stay byte-identical.
     **/
    struct color_config {
        static inline bool color_enabled = false;
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

    /** @brief RAII: emit @p spec's on-escape now and its off-escape at end of
     *  scope, so a run of put()s renders colored:
     *  @code
     *    { color_guard g(sink, spec); sink.put("+"); sink.put(name); }
     *  @endcode
     *  Both escapes are gated by color_config::color_enabled and spec != none.
     **/
    class color_guard {
    public:
        color_guard(PpSink & sink, const color_spec_type & spec)
            : sink_{sink}, spec_{spec},
              on_{color_config::color_enabled
                  && (spec.encoding() != color_encoding::none)}
        {
            if (on_)
                spec_.fg_on(sink_);
        }
        ~color_guard() {
            if (on_)
                spec_.fg_off(sink_);
        }
        color_guard(const color_guard &) = delete;
        color_guard & operator=(const color_guard &) = delete;

    private:
        PpSink & sink_;
        color_spec_type spec_;
        bool on_;
    };

    /** color a single string @p text with @p spec **/
    inline void with_color(PpSink & sink, const color_spec_type & spec, std::string_view text) {
        color_guard g(sink, spec);
        sink.put(text);
    }
} /*namespace xo::pp*/

/* end color.hpp */
