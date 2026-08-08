/** @file pad.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  pad(n) / pad(n, ch) / spaces(n): a run of @p n copies of a character.
 *  The ppsink replacement for legacy xo-indentlog print/pad.hpp.
 *
 *  @code
 *    sink.pp(pad(8));           // 8 spaces
 *    sink.pp(pad(16, '-'));     // ----------------
 *    sink.pp(spaces(4));        // 4 spaces
 *  @endcode
 *
 *  Mostly used to hand-indent a nested rendering, e.g. xo-ordinaltree's tree
 *  dumps.  NB a PpSink can do that itself -- begin()/split()/end() carry a
 *  running indent and adapt to the right margin, which manual padding cannot.
 *  Prefer the sink's own structure for new code; this exists so existing
 *  hand-indented printers can migrate without being rewritten at the same time.
 *
 *  This header is ostream-free; see pad_ostream.hpp to stream a pad directly
 *  to an ostream.
 **/

#pragma once

#include "Prettifier.hpp"
#include <cstdint>

namespace xo::pp {
    /** @brief a run of @ref n_pad copies of @ref pad_char **/
    class pad_impl {
    public:
        constexpr pad_impl(std::uint32_t n, char pad_char) noexcept
            : n_pad_{n}, pad_char_{pad_char} {}

        constexpr std::uint32_t n_pad() const noexcept { return n_pad_; }
        constexpr char pad_char() const noexcept { return pad_char_; }

    private:
        /** number of characters to write **/
        std::uint32_t n_pad_ = 0;
        /** the character to repeat **/
        char pad_char_ = '\0';
    };

    /** @p n copies of @p pad_char (default: space) **/
    inline constexpr pad_impl
    pad(std::uint32_t n, char pad_char = ' ') noexcept {
        return pad_impl(n, pad_char);
    }

    /** @p n spaces **/
    inline constexpr pad_impl
    spaces(std::uint32_t n) noexcept {
        return pad_impl(n, ' ');
    }

    /** render a pad_impl as a single token.
     *
     *  One put() per chunk rather than per character: a padding run is
     *  indivisible -- the pretty-printer must not break a line inside it.
     **/
    template <>
    struct Prettifier<pad_impl> {
        static void print(PpSink & sink, pad_impl x) {
            /* chunked so an enormous n does not need an enormous buffer */
            constexpr std::uint32_t c_chunk_z = 64;

            char buf[c_chunk_z];
            std::uint32_t remaining = x.n_pad();

            while (remaining > 0) {
                const std::uint32_t z = (remaining < c_chunk_z) ? remaining : c_chunk_z;

                for (std::uint32_t i = 0; i < z; ++i)
                    buf[i] = x.pad_char();

                sink.put(std::string_view(buf, z));

                remaining -= z;
            }
        }
    };
} /*namespace xo::pp*/

/* end pad.hpp */
