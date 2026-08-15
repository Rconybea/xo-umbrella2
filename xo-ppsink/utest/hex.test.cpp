/** @file hex.test.cpp
 *
 *  exercise hex_view + hexstyle from xo/ppsink/hex.hpp, and the ostream
 *  bridge in hex_ostream.hpp.
 *
 *  Three kinds of assertion:
 *
 *  - via FlatSink: the flat *text*.  FlatSink renders a split as its flat
 *    spaces, so this is also what the legacy xo::hex_view produced -- these
 *    cases pin byte-compatibility with what they replace.
 *  - via MarkSink: the *token stream*, which is where the 16-byte row
 *    grouping actually lives.  Flat text cannot distinguish put(" ") from
 *    split(1); only the token stream can, and the difference is the whole
 *    point (a split is a break opportunity, a space is not).
 *  - via operator<<: the ostream bridge.
 *
 *  See xo-indentlog2/utest/hex.test.cpp for the rendered, actually-wrapped
 *  form against a real PrettySink.
 **/

#include <xo/ppsink/hex.hpp>
#include <xo/ppsink/hex_ostream.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include "MarkSink.hpp"
#include <catch2/catch.hpp>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace ut {
    using xo::pp::FlatSink;
    using xo::pp::hex_view;
    using xo::pp::hexstyle;
    using std::stringstream;

    namespace {
        /** flat rendering of @p x, as a reader sees it on one line **/
        std::string
        flat(const hex_view & x) {
            stringstream ss;
            FlatSink sink(ss.rdbuf());
            sink.pp(x);
            return ss.str();
        }

        /** token stream of @p x: put()s verbatim, structure as markers **/
        std::string
        marks(const hex_view & x) {
            stringstream ss;
            MarkSink sink(ss);
            sink.pp(x);
            return ss.str();
        }

        /** how many times @p needle occurs in @p s (non-overlapping) **/
        std::size_t
        count_of(std::string_view s, std::string_view needle) {
            std::size_t n = 0;

            for (std::size_t p = s.find(needle);
                 p != std::string_view::npos;
                 p = s.find(needle, p + needle.size()))
            {
                ++n;
            }

            return n;
        }

        /** a buffer of @p z bytes, values 0, 1, 2, .. (mod 256) **/
        std::vector<unsigned char>
        counting_buf(std::size_t z) {
            std::vector<unsigned char> v(z);

            for (std::size_t i = 0; i < z; ++i)
                v[i] = static_cast<unsigned char>(i);

            return v;
        }
    } /*namespace*/

    TEST_CASE("hex-flat-bare", "[hex]") {
        std::string_view s = "hello";

        REQUIRE(flat(hex_view(s)) == "[68 65 6c 6c 6f]");
    }

    TEST_CASE("hex-flat-with-char", "[hex]") {
        std::string_view s = "hello";

        REQUIRE(flat(hex_view(s, hexstyle::with_char))
                == "[68(h) 65(e) 6c(l) 6c(l) 6f(o)]");
    }

    TEST_CASE("hex-nonprintable-renders-question-mark", "[hex]") {
        /* 0x00 and 0x7f are both outside the printable ascii range;
         * 0x20 (space) is inside it.
         */
        const unsigned char raw[] = { 0x00, 0x20, 0x7f, 0x80 };

        REQUIRE(flat(hex_view(std::span<const unsigned char>(raw, 4),
                              hexstyle::with_char))
                == "[00(?) 20( ) 7f(?) 80(?)]");
    }

    TEST_CASE("hex-empty-range", "[hex]") {
        std::string_view s = "";

        REQUIRE(flat(hex_view(s)) == "[]");
    }

    TEST_CASE("hex-accepts-every-range-form", "[hex]") {
        /* the point of constraining on contiguous_range: one ctor covers the
         * std types, and (elsewhere) xo::mm::span / xo::scm::span too.
         */
        const char cbuf[] = "hi";
        std::string_view sv = "hi";
        std::vector<char> vec = { 'h', 'i' };
        const unsigned char ubuf[] = { 0x68, 0x69 };
        std::string str = "hi";

        const std::string expected = "[68 69]";

        REQUIRE(flat(hex_view(sv)) == expected);
        REQUIRE(flat(hex_view(vec)) == expected);
        REQUIRE(flat(hex_view(str)) == expected);
        REQUIRE(flat(hex_view(std::span<const unsigned char>(ubuf, 2))) == expected);
        /* legacy (lo, hi) pointer-pair form */
        REQUIRE(flat(hex_view(cbuf, cbuf + 2)) == expected);
    }

    TEST_CASE("hex-token-stream", "[hex]") {
        /* pins the three shape rules that flat text cannot see:
         *   - "[" is emitted BEFORE begin(), so it stays on the opening line
         *   - there is NO leading split (legacy renders "[68 ..", not "[ 68")
         *   - "]" is emitted BEFORE end(), so it counts toward the group width
         * and that within a row the separator is a plain space, not a split.
         */
        std::string_view s = "abc";

        REQUIRE(marks(hex_view(s)) == "[<G>61 62 63]</G>");
    }

    TEST_CASE("hex-splits-only-at-row-boundaries", "[hex]") {
        /* a split is a break opportunity; a space is not.  Splitting between
         * every byte would let a line break mid-row, which is exactly the
         * layout this grouping exists to prevent.
         */
        REQUIRE(hex_view::c_bytes_per_row == 16);

        SECTION("under one row: no split at all") {
            auto buf = counting_buf(15);

            REQUIRE(count_of(marks(hex_view(buf)), "<S ") == 0);
        }

        SECTION("exactly one row: still no split") {
            auto buf = counting_buf(16);

            REQUIRE(count_of(marks(hex_view(buf)), "<S ") == 0);
        }

        SECTION("one byte past a row: exactly one split") {
            auto buf = counting_buf(17);
            std::string m = marks(hex_view(buf));

            REQUIRE(count_of(m, "<S ") == 1);
            /* split(1): one space when flat, and no extra indent offset */
            REQUIRE(count_of(m, "<S 1,0>") == 1);
            /* the split falls between byte 15 (0f) and byte 16 (10) */
            REQUIRE(m.find("0f<S 1,0>10") != std::string::npos);
        }

        SECTION("three rows: one split per row boundary") {
            auto buf = counting_buf(33);

            REQUIRE(count_of(marks(hex_view(buf)), "<S 1,0>") == 2);
        }
    }

    TEST_CASE("hex-flat-crosses-row-boundary-as-a-space", "[hex]") {
        /* FlatSink renders a split as its flat spaces, so a long range comes
         * out as one run -- byte-identical to what legacy xo::hex_view did.
         */
        auto buf = counting_buf(17);
        std::string f = flat(hex_view(buf));

        REQUIRE(f.find('\n') == std::string::npos);
        REQUIRE(f.substr(0, 9) == "[00 01 02");
        /* single space at the row boundary, same as everywhere else */
        REQUIRE(f.find("0f 10") != std::string::npos);
    }

    TEST_CASE("hex-ostream-bridge", "[hex]") {
        std::string_view s = "hello";
        stringstream ss;

        ss << hex_view(s, hexstyle::with_char);

        REQUIRE(ss.str() == "[68(h) 65(e) 6c(l) 6c(l) 6f(o)]");
    }
} /*namespace ut*/

/* end hex.test.cpp */
