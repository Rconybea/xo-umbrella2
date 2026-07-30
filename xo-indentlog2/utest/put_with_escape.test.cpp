/** @file put_with_escape.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Exercise PpState::put_with_escape() through PrettySink: expansion into a
 *  PpStringToken, and the token's participation in layout decisions.
 **/

#include "print/PrettySink.hpp"
#include <xo/ppsink/escape.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <string>
#include <string_view>
#include <cstdint>

namespace ut {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::Escape;
    using xo::mm::ArenaConfig;
    using std::string;
    using std::string_view;

    namespace {
        PpConfig
        escape_test_config(std::uint32_t soft_right_margin) {
            ArenaConfig logbuf_cfg { .name_ = "utest.put_with_escape",
                                     .size_ = 256*1024 };

            PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg);

            if (soft_right_margin > 0)
                cfg = cfg.with_soft_right_margin(soft_right_margin);

            return cfg;
        }

        /** put @p s through a fresh PrettySink via put_with_escape **/
        string
        pp_escape(string_view s, bool quote_flag) {
            PrettySink pp(escape_test_config(0));

            pp.put_with_escape(s, quote_flag);

            return string(pp.output());
        }
    } /*namespace*/

    TEST_CASE("ppstate-put-with-escape-unquoted", "[put_with_escape]") {
        /* same rules as FlatSink, but arriving via a PpStringToken */
        REQUIRE(pp_escape("plain", false) == "plain");
        REQUIRE(pp_escape("a\nb", false) == "a\\nb");
        REQUIRE(pp_escape("a\"b", false) == "a\\\"b");
        REQUIRE(pp_escape("a\\b", false) == "a\\\\b");
        REQUIRE(pp_escape("a\tb", false) == "a\\x09b");
        REQUIRE(pp_escape("", false) == "");
    }

    TEST_CASE("ppstate-put-with-escape-quoted", "[put_with_escape]") {
        REQUIRE(pp_escape("plain", true) == "\"plain\"");
        REQUIRE(pp_escape("a b", true) == "\"a b\"");
        REQUIRE(pp_escape("a\nb", true) == "\"a\\nb\"");
        /* an embedded quote is escaped, so it can't close the string early */
        REQUIRE(pp_escape("a\"b", true) == "\"a\\\"b\"");
        /* empty string still gets its delimiters */
        REQUIRE(pp_escape("", true) == "\"\"");
    }

    TEST_CASE("ppstate-put-with-escape-esc-byte", "[put_with_escape]") {
        /* ESC must arrive escaped.  Left raw, count_visible_chars() would read
         * it as the start of a zero-width color escape and undercount the
         * token's visible length -- which is what the next test relies on.
         */
        REQUIRE(pp_escape("\x1b", false) == "\\x1b");
        REQUIRE(pp_escape("\x1b[31m", true) == "\"\\x1b[31m\"");
    }

    TEST_CASE("ppstate-put-with-escape-counts-escaped-width", "[put_with_escape]") {
        /* An escaped token must be measured at its *escaped* width, so the
         * pretty printer's fits decision is made on what actually prints.
         *
         * Each token here renders as "\x1b" -- 6 visible characters -- so the
         * group needs 12 columns and cannot fit a margin of 8: the split must
         * become a newline.  If the ESC byte went in raw, the tokens would
         * measure 3 wide, the group would "fit", and no break would happen.
         */
        PrettySink pp(escape_test_config(8));

        pp.begin();
        pp.put_with_escape("\x1b", true);
        pp.split();
        pp.put_with_escape("\x1b", true);
        pp.end();

        REQUIRE(string(pp.output()) == "\"\\x1b\"\n  \"\\x1b\"");
    }

    TEST_CASE("ppstate-put-with-escape-is-one-token", "[put_with_escape]") {
        /* The whole expansion, quotes included, has to land in a single token:
         * otherwise the printer could break a line inside the escaped string,
         * or between a quote and what it quotes.  Squeeze it well past the
         * margin and require the rendering stay contiguous.
         */
        PrettySink pp(escape_test_config(4));

        pp.begin();
        pp.put_with_escape("hello world", true);
        pp.end();

        string out(pp.output());

        REQUIRE(out == "\"hello world\"");
        REQUIRE(out.find('\n') == string::npos);
    }

    TEST_CASE("ppstate-put-with-escape-long-token", "[put_with_escape]") {
        /* A token whose expansion is much larger than its input: exercises
         * sizing off Escape::str_size() rather than the input length.
         */
        constexpr std::size_t c_n = 64;

        string input(c_n, '\x1b');

        string expected = "\"";
        for (std::size_t i = 0; i < c_n; ++i)
            expected += "\\x1b";
        expected += "\"";

        string out = pp_escape(input, true);

        REQUIRE(out.size() == 1 + c_n * Escape::c_max_char_expand + 1);
        REQUIRE(out == expected);
    }
} /*namespace ut*/

/* end put_with_escape.test.cpp */
