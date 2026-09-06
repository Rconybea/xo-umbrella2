/** @file hex.test.cpp
 *
 *  exercise hex_view (xo/ppsink/hex.hpp) through the real line-breaking sink.
 *
 *  xo-ppsink's own hex tests can only pin the flat text and the token stream
 *  -- FlatSink's split emits its flat spaces and never breaks, and PrettySink
 *  lives here.  So this is the only place the *rendered wrapped form* can be
 *  asserted: that a long dump breaks at 16-byte row boundaries, and at what
 *  column the continuation rows land.
 *
 *  Two things these tests exist to catch:
 *
 *  1. Indent double-counting.  begin(offset) and split(spaces,offset) COMPOUND
 *     in PpState (begin adds to the running indent; a split's break target is
 *     running-indent + its own offset).  A printer passing the indent to both
 *     wraps to twice the intended column, and nothing in the flat case reveals
 *     it.  Continuation rows must land at column 2.
 *  2. Row atomicity.  Bytes within a row are separated by put(" "), not
 *     split(), so a row is unbreakable -- see the narrow-margin case, which
 *     deliberately asserts that output OVERRUNS the margin rather than
 *     splitting a row.
 *
 *  Also covers the one hex_view ctor arm xo-ppsink cannot reach on its own:
 *  xo::mm::span, which satisfies detail::byte_range structurally without
 *  ppsink ever naming (or depending on) xo-arena.
 **/

#include <xo/ppsink/hex.hpp>
#include <xo/ppsink/pretty.hpp> /* PpSink::pp */
#include "print/PrettySink.hpp"
#include "print/toppstr.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <xo/arena/span.hpp>
#include <catch2/catch.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace ut {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::hex;
    using xo::pp::hex_view;
    using xo::pp::hexprefix;
    using xo::pp::hexstyle;
    using xo::mm::ArenaConfig;

    namespace {
        /** render @p x at right margin @p margin.
         *
         *  NB the arena name must be unique per call: two PrettySinks sharing
         *  an ArenaConfig name interfere, and the symptom is wrong indentation
         *  in whichever case runs second -- which looks exactly like a bug in
         *  the printer under test.  Hence the counter.
         **/
        std::string
        render(std::uint32_t margin, const hex_view & x)
        {
            return toppstr(PpConfig::scratch_plain(margin), x);
        }

        /** render a single byte @p x -- same sink setup as the hex_view
         *  overload above.  Margin is irrelevant for a scalar (it is one
         *  unbreakable token), so callers pass a wide one.
         **/
        std::string
        render(std::uint32_t margin, const hex & x)
        {
            return toppstr(PpConfig::scratch_plain(margin), x);
        }

        /** a buffer of @p z bytes, values 0, 1, 2, .. **/
        std::vector<unsigned char>
        counting_buf(std::size_t z) {
            std::vector<unsigned char> v(z);

            for (std::size_t i = 0; i < z; ++i)
                v[i] = static_cast<unsigned char>(i);

            return v;
        }

        /* the three rows a 40-byte counting_buf produces */
        constexpr std::string_view c_row0 = "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f";
        constexpr std::string_view c_row1 = "10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f";
        constexpr std::string_view c_row2 = "20 21 22 23 24 25 26 27";
    } /*namespace*/

    TEST_CASE("hex-fits-one-line", "[hex]") {
        auto buf = counting_buf(20);

        /* wide margin: the split at the row boundary renders as one space,
         * so this is byte-identical to what legacy xo::hex_view produced.
         */
        REQUIRE(render(200, hex_view(buf))
                == "[00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13]");
    }

    TEST_CASE("hex-breaks-at-row-boundary", "[hex]") {
        auto buf = counting_buf(20);

        /* continuation row at column 2 -- NOT 4.  This is the direct
         * regression test for begin()/split() indent compounding.
         */
        REQUIRE(render(40, hex_view(buf))
                == std::string("[").append(c_row0)
                   .append("\n  10 11 12 13]"));
    }

    TEST_CASE("hex-breaks-every-row", "[hex]") {
        auto buf = counting_buf(40);

        /* 16 + 16 + 8.  Every continuation row lands at the same column 2;
         * the indent does not accumulate per row.
         */
        REQUIRE(render(40, hex_view(buf))
                == std::string("[").append(c_row0)
                   .append("\n  ").append(c_row1)
                   .append("\n  ").append(c_row2)
                   .append("]"));
    }

    TEST_CASE("hex-row-is-atomic", "[hex]") {
        auto buf = counting_buf(40);

        /* margin 20 is far narrower than a 47-column row, but there are no
         * split points *within* a row, so the output is identical to the
         * margin-40 rendering and simply overruns.  That is deliberate: a hex
         * row broken mid-row is harder to read than one that overruns.
         */
        REQUIRE(render(20, hex_view(buf)) == render(40, hex_view(buf)));
    }

    TEST_CASE("hex-with-char-style", "[hex]") {
        std::string_view s = "hello";

        REQUIRE(render(200, hex_view(s, hexstyle::with_char))
                == "[68(h) 65(e) 6c(l) 6c(l) 6f(o)]");
    }

    TEST_CASE("hex-accepts-xo-arena-span", "[hex]") {
        /* the ctor arm xo-ppsink's own tests cannot exercise: xo::mm::span
         * satisfies detail::byte_range through begin()/end()/data()/size(),
         * with no dependency from ppsink on xo-arena.
         */
        char raw[] = "hi";
        xo::mm::span<char> s(raw, raw + 2);

        REQUIRE(render(200, hex_view(s)) == "[68 69]");
    }
    /* ---------------- hex (single byte) ---------------- */

    TEST_CASE("hex-byte-bare", "[hex]") {
        /* two lowercase digits, no "0x" prefix -- matching hex_view and
         * legacy xo::hex.  Leading zero is preserved.
         */
        REQUIRE(render(200, hex(0xfd)) == "fd");
        REQUIRE(render(200, hex(0x00)) == "00");
        REQUIRE(render(200, hex(0x0f)) == "0f");
        REQUIRE(render(200, hex(0xff)) == "ff");
    }

    TEST_CASE("hex-byte-with-char", "[hex]") {
        /* printable codes show the character; non-printable show '?'.
         * 0x4f is 'O' -- the example from legacy hex.hpp's docstring.
         */
        REQUIRE(render(200, hex(0x4f, hexstyle::with_char)) == "4f(O)");
        REQUIRE(render(200, hex(0x20, hexstyle::with_char)) == "20( )");
        REQUIRE(render(200, hex(0x1f, hexstyle::with_char)) == "1f(?)");
        REQUIRE(render(200, hex(0x7f, hexstyle::with_char)) == "7f(?)");
    }

    TEST_CASE("hex-byte-agrees-with-hex-view", "[hex]") {
        /* hex and hex_view share detail::put_hex_byte(), so a byte must
         * render identically alone and inside a dump.  Pinned because the
         * two have separate Prettifiers and could drift.
         */
        for (int i = 0; i < 256; ++i) {
            auto uc = static_cast<std::uint8_t>(i);

            /* a one-byte dump is just the byte in brackets */
            REQUIRE(render(200, hex_view(&uc, &uc + 1))
                    == "[" + render(200, hex(uc)) + "]");

            REQUIRE(render(200, hex_view(&uc, &uc + 1, hexstyle::with_char))
                    == "[" + render(200, hex(uc, hexstyle::with_char)) + "]");
        }
    }

    TEST_CASE("hex-byte-qualified", "[hex]") {
        /* hexprefix::qualified adds the 0x radix prefix.  Composes with
         * with_char, and either argument order spells the same thing.
         */
        REQUIRE(render(200, hex(0xfd, hexprefix::qualified)) == "0xfd");
        REQUIRE(render(200, hex(0x00, hexprefix::qualified)) == "0x00");

        REQUIRE(render(200, hex(0x4f, hexprefix::qualified, hexstyle::with_char))
                == "0x4f(O)");
        REQUIRE(render(200, hex(0x4f, hexstyle::with_char, hexprefix::qualified))
                == "0x4f(O)");

        /* plain is the default, and stays the default */
        REQUIRE(render(200, hex(0xfd)) == "fd");
        REQUIRE(render(200, hex(0xfd, hexprefix::plain)) == "fd");
    }

    TEST_CASE("hex-view-never-qualifies", "[hex]") {
        /* the prefix is scalar-only: a dump already says "these are bytes",
         * so hex_view has no prefix option and must not grow one by accident
         */
        std::string_view s = "hi";

        REQUIRE(render(200, hex_view(s)) == "[68 69]");
    }

    TEST_CASE("hex-byte-is-one-token", "[hex]") {
        /* a byte must never be split across lines.  Even at an absurdly
         * narrow margin the token stays intact -- same guarantee the
         * row-atomicity case makes for hex_view.
         */
        REQUIRE(render(1, hex(0xab)) == "ab");
        REQUIRE(render(1, hex(0x4f, hexstyle::with_char)) == "4f(O)");
        REQUIRE(render(1, hex(0x4f, hexprefix::qualified, hexstyle::with_char))
                == "0x4f(O)");
    }
} /*namespace ut*/

/* end hex.test.cpp */
