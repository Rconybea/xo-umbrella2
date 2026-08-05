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
#include <xo/arena/ArenaConfig.hpp>
#include <xo/arena/span.hpp>
#include <catch2/catch.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace ut {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::hex_view;
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
            static int seq = 0;

            ArenaConfig logbuf_cfg {
                .name_ = "utest.hex." + std::to_string(++seq),
                .size_ = 64*1024 };

            PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg)
                                     .with_soft_right_margin(margin);

            PrettySink pp(cfg, nullptr);

            pp.pp(x);

            return std::string(pp.output());
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
} /*namespace ut*/

/* end hex.test.cpp */
