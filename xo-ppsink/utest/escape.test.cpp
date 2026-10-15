/** @file escape.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Exercise xo::pp::Escape (the escape rules) and
 *  FlatSink::put_with_escape (the ostream-side application of them).
 **/

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/escape.hpp>
#include <xo/ppsink/quoted_char_ostream.hpp>
#include <xo/ppsink/quoted_ostream.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>

namespace ut {
    using xo::pp::Escape;
    using xo::pp::FlatSink;
    using xo::pp::PpStyle;
    using xo::pp::escapestyle;
    using xo::pp::quoted_char;
    using xo::pp::quot;
    using xo::pp::unq;
    using std::string;
    using std::string_view;

    /* Escape is usable at compile time */
    static_assert(Escape::str_size("").size == 0);
    static_assert(Escape::str_size("abc").size == 3);
    static_assert(Escape::str_size("abc").escape_flag == false);
    static_assert(Escape::str_size("a\nb").size == 4);
    static_assert(Escape::str_size("a\nb").escape_flag == true);
    /* tab/backspace/formfeed have short forms -> 2 bytes, not \xNN.
     * VT (0x0b) deliberately does not, so it still costs the full \xNN width.
     */
    static_assert(Escape::str_size("\t").size == 2);
    static_assert(Escape::str_size("\b").size == 2);
    static_assert(Escape::str_size("\f").size == 2);
    static_assert(Escape::str_size("\v").size == Escape::fallback_expand(escapestyle::display));
    /* json spells the same set: short forms unchanged, \u00NN for the rest */
    static_assert(Escape::fallback_expand(escapestyle::display) == 4);   /* \xNN */
    static_assert(Escape::fallback_expand(escapestyle::json) == 6);      /* \u00NN */
    static_assert(Escape::str_size("\t", escapestyle::json).size == 2);
    static_assert(Escape::str_size("\v", escapestyle::json).size == 6);
    /* the bound covers every vocabulary */
    static_assert(Escape::c_max_char_expand >= Escape::fallback_expand(escapestyle::display));
    static_assert(Escape::c_max_char_expand >= Escape::fallback_expand(escapestyle::json));
    /* escape_flag is the "needs quotes" predicate, so these are set with
     * nothing escaped
     */
    static_assert(Escape::str_size("").escape_flag == true);
    static_assert(Escape::str_size("a b").escape_flag == true);
    static_assert(Escape::str_size("a b").size == 3);

    /** expand @p input per Escape and return the result.
     *
     *  Also pins the invariant the two methods have to share: str_copy writes
     *  exactly str_size().size bytes.
     **/
    static string
    escape_str(string_view input, escapestyle style = escapestyle::display) {
        auto [esc_z, esc_flag] = Escape::str_size(input, style);

        string buf(esc_z, '\0');
        char * end = Escape::str_copy(input, buf.data(), style);

        REQUIRE(static_cast<std::uint32_t>(end - buf.data()) == esc_z);
        /* nothing to check against esc_flag here: it reports whether quotes
         * are needed, not whether any byte changed (see size_result docs)
         */
        (void)esc_flag;

        return buf;
    }

    TEST_CASE("escape-passthrough", "[escape]") {
        /* nothing in these needs escaping, so they survive byte-identical.
         * Note space is NOT escaped -- an embedded space is a reason to quote,
         * which is the caller's decision, not this layer's.
         */
        REQUIRE(escape_str("") == "");
        REQUIRE(escape_str("abc") == "abc");
        REQUIRE(escape_str("a b") == "a b");
        REQUIRE(escape_str("/:;,()[]{}") == "/:;,()[]{}");
    }

    TEST_CASE("escape-quote-predicate", "[escape]") {
        /* escape_flag drives unq(): true iff bare would be ambiguous */
        auto needs_quotes = [](string_view s) {
            return Escape::str_size(s).escape_flag;
        };

        /* unambiguous bare */
        REQUIRE(needs_quotes("abc") == false);
        REQUIRE(needs_quotes("a-b_c.d") == false);
        REQUIRE(needs_quotes("\xc3\xa9") == false);

        /* set because something was escaped */
        REQUIRE(needs_quotes("a\nb") == true);
        REQUIRE(needs_quotes("a\"b") == true);
        REQUIRE(needs_quotes("a\\b") == true);
        REQUIRE(needs_quotes("a\tb") == true);

        /* set with nothing escaped: a space would read as a separator, and
         * the empty string would print as nothing at all
         */
        REQUIRE(needs_quotes("a b") == true);
        REQUIRE(needs_quotes(" ") == true);
        REQUIRE(needs_quotes("") == true);
    }

    TEST_CASE("escape-short-forms", "[escape]") {
        /* the seven escapes with a two-character form */
        REQUIRE(escape_str("\\") == "\\\\");
        REQUIRE(escape_str("\"") == "\\\"");
        REQUIRE(escape_str("\n") == "\\n");
        REQUIRE(escape_str("\r") == "\\r");
        REQUIRE(escape_str("\t") == "\\t");
        REQUIRE(escape_str("\b") == "\\b");
        REQUIRE(escape_str("\f") == "\\f");

        REQUIRE(escape_str("a\"b") == "a\\\"b");
        REQUIRE(escape_str("a\\b") == "a\\\\b");
        REQUIRE(escape_str("a\nb") == "a\\nb");
        REQUIRE(escape_str("a\tb") == "a\\tb");
    }

    TEST_CASE("escape-control-chars", "[escape]") {
        /* control characters WITHOUT a short form -> \xNN, lowercase hex.
         * VT (0x0b) is the neighbour of \t/\f that deliberately did NOT get a
         * short form, so it pins that the short-form set stayed bounded.
         */
        REQUIRE(escape_str("\v") == "\\x0b");
        REQUIRE(escape_str(string_view("\0", 1)) == "\\x00");
        REQUIRE(escape_str("\x7f") == "\\x7f");

        /* ESC specifically: left raw it would look like the start of a color
         * escape to PpState::count_visible_chars, and the token's visible
         * length would undercount.
         */
        REQUIRE(escape_str("\x1b") == "\\x1b");
        REQUIRE(escape_str("\x1b[31m") == "\\x1b[31m");
    }

    TEST_CASE("escape-json-control-chars", "[escape][json]") {
        /* json: the characters display escapes, in JSON's spelling (RFC 8259
         * section 7).  No \x in JSON; a strict parser rejects it.
         */
        REQUIRE(escape_str(string_view("\0", 1), escapestyle::json) == "\\u0000");
        REQUIRE(escape_str("\v", escapestyle::json) == "\\u000b");
        REQUIRE(escape_str("\x1b", escapestyle::json) == "\\u001b");
        REQUIRE(escape_str("\x1f", escapestyle::json) == "\\u001f");
        /* DEL may be raw in JSON, but raw is invisible; escaped stays visible */
        REQUIRE(escape_str("\x7f", escapestyle::json) == "\\u007f");

        /* short forms and quote/backslash: the same in both vocabularies */
        REQUIRE(escape_str("a\nb\tc", escapestyle::json) == "a\\nb\\tc");
        REQUIRE(escape_str("q\"b\\", escapestyle::json) == "q\\\"b\\\\");

        /* UTF-8 passes through: JSON permits it raw */
        REQUIRE(escape_str("\xc3\xa9", escapestyle::json) == "\xc3\xa9");
    }

    TEST_CASE("escape-leaves-utf8-alone", "[escape]") {
        /* bytes >= 0x80 are not control characters and must pass through, so
         * UTF-8 survives.  (Guards against a signed-char comparison treating
         * high bytes as < 0x20.)
         */
        REQUIRE(escape_str("\xc3\xa9") == "\xc3\xa9");             /* e-acute */
        REQUIRE(escape_str("\xe2\x86\x92") == "\xe2\x86\x92");     /* arrow */
        REQUIRE(Escape::str_size("\xc3\xa9").escape_flag == false);
    }

    TEST_CASE("flatsink-put-with-escape", "[escape][flatsink]") {
        auto expand = [](string_view s, bool quote_flag) {
            std::stringstream ss;
            FlatSink sink(ss.rdbuf());
            sink.put_with_escape(s, quote_flag);
            return ss.str();
        };

        /* quote_flag=false: escapes only, no delimiters */
        REQUIRE(expand("a\nb", false) == "a\\nb");
        REQUIRE(expand("plain", false) == "plain");
        REQUIRE(expand("", false) == "");

        /* quote_flag=true: same expansion, wrapped in Escape::c_quote */
        REQUIRE(expand("a\nb", true) == "\"a\\nb\"");
        REQUIRE(expand("plain", true) == "\"plain\"");
        REQUIRE(expand("a b", true) == "\"a b\"");
        /* empty string still gets its delimiters */
        REQUIRE(expand("", true) == "\"\"");
        /* an embedded quote is escaped, so it can't close the string early */
        REQUIRE(expand("a\"b", true) == "\"a\\\"b\"");
    }

    TEST_CASE("quoted-char", "[escape][quoted_char]") {
        /* quoted_char goes through put_with_escape with quote_flag=false, so it
         * follows the Escape rules and adds no surrounding quotes.
         */
        auto q = [](char ch) {
            std::stringstream ss;
            ss << quoted_char(ch);
            return ss.str();
        };

        /* printable characters are unchanged, and never wrapped in quotes */
        REQUIRE(q('x') == "x");
        REQUIRE(q(' ') == " ");

        /* short forms */
        REQUIRE(q('\n') == "\\n");
        REQUIRE(q('\r') == "\\r");
        REQUIRE(q('\\') == "\\\\");
        REQUIRE(q('"') == "\\\"");

        /* ESC as \x1b -- the legacy xo-indentlog quoted_char emitted octal
         * \033 here, and passed tab/NUL through raw.  ppsink uses one escape
         * vocabulary instead (see quoted_char.hpp).
         */
        REQUIRE(q('\x1b') == "\\x1b");
        REQUIRE(q('\t') == "\\t");
        REQUIRE(q('\b') == "\\b");
        REQUIRE(q('\f') == "\\f");
        REQUIRE(q('\0') == "\\x00");
    }

    TEST_CASE("quot-and-unq", "[escape][quoted]") {
        auto s_quot = [](string_view s) {
            std::stringstream ss;
            ss << quot(s);
            return ss.str();
        };
        auto s_unq = [](string_view s) {
            std::stringstream ss;
            ss << unq(s);
            return ss.str();
        };

        /* quot() always quotes, even when bare would be unambiguous */
        REQUIRE(s_quot("abc") == "\"abc\"");
        REQUIRE(s_quot("a b") == "\"a b\"");
        REQUIRE(s_quot("") == "\"\"");
        REQUIRE(s_quot("a\nb") == "\"a\\nb\"");
        REQUIRE(s_quot("a\"b") == "\"a\\\"b\"");

        /* unq() quotes only when bare would be ambiguous */
        REQUIRE(s_unq("abc") == "abc");
        REQUIRE(s_unq("\xc3\xa9") == "\xc3\xa9");
        REQUIRE(s_unq("a b") == "\"a b\"");
        REQUIRE(s_unq("") == "\"\"");
        REQUIRE(s_unq("a\nb") == "\"a\\nb\"");
        REQUIRE(s_unq("a\"b") == "\"a\\\"b\"");

        /* both accept const char * and std::string via string_view */
        REQUIRE(s_unq(string("a b")) == "\"a b\"");
        REQUIRE(s_quot("lit") == "\"lit\"");
    }

    TEST_CASE("flatsink-put-with-escape-spans-buffer", "[escape][flatsink]") {
        /* FlatSink expands through a fixed stack buffer, flushing as it fills.
         * Use an input whose expansion is several times that buffer, entirely
         * of maximum-width expansions, so the flush path runs repeatedly and
         * the closing quote lands after a flush boundary.
         */
        constexpr std::size_t c_n = 400;

        string input(c_n, '\x1b');

        string expected = "\"";
        for (std::size_t i = 0; i < c_n; ++i)
            expected += "\\x1b";
        expected += "\"";

        std::stringstream ss;
        FlatSink sink(ss.rdbuf());
        sink.put_with_escape(input, true);

        REQUIRE(ss.str().size() == 1 + c_n * Escape::fallback_expand(escapestyle::display) + 1);
        REQUIRE(ss.str() == expected);
    }

    /* a json-styled FlatSink: the sink's style picks the vocabulary */
    TEST_CASE("flatsink-put-with-escape-json", "[escape][flatsink][json]") {
        std::stringstream ss;
        FlatSink sink(PpStyle::json(), ss.rdbuf());
        sink.put_with_escape(string_view("a\0b", 3), true);

        REQUIRE(ss.str() == "\"a\\u0000b\"");
    }

    /* json's 6-byte expansion through the flush path: the buffer reserve
     * must cover the widest vocabulary
     */
    TEST_CASE("flatsink-put-with-escape-json-spans-buffer", "[escape][flatsink][json]") {
        constexpr std::size_t c_n = 400;

        string input(c_n, '\x1b');

        string expected = "\"";
        for (std::size_t i = 0; i < c_n; ++i)
            expected += "\\u001b";
        expected += "\"";

        std::stringstream ss;
        FlatSink sink(PpStyle::json(), ss.rdbuf());
        sink.put_with_escape(input, true);

        REQUIRE(ss.str().size() == 1 + c_n * Escape::fallback_expand(escapestyle::json) + 1);
        REQUIRE(ss.str() == expected);
    }

    /* quot() renders through the sink it reaches, so it escapes per that
     * sink's style
     */
    TEST_CASE("quot-follows-sink-style", "[escape][quoted][json]") {
        std::stringstream ss;
        FlatSink sink(PpStyle::json(), ss.rdbuf());
        sink.pp(quot(string_view("a\x7f", 2)));

        REQUIRE(ss.str() == "\"a\\u007f\"");
    }
} /*namespace ut*/

/* end escape.test.cpp */
