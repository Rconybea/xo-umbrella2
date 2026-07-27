/** @file scope.test.cpp **/

#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <cctype>

namespace ut {

    using xo::pp::scope;
    using xo::pp::scope_config;
    using xo::pp::FlatSink;
    using xo::pp::ThreadLogState;
    using std::stringstream;

    TEST_CASE("scope-indent-flat", "[scope]") {
    /* capture scope output into a stringstream via a FlatSink */
    stringstream ss;
    FlatSink sink(ss);
    ThreadLogState::log_set_sink(&sink);

    {
        scope outer("outer");
        outer.log("hello");
        {
            scope inner("inner");
            inner.log("world", 42);
        }
        outer.log("bye");
    }

    ThreadLogState::log_set_sink(nullptr);   /* restore default (clog) */

    REQUIRE(ss.str() ==
            "+outer\n"
            "  hello\n"
            "  +inner\n"
            "    world42\n"
            "  -inner\n"
            "  bye\n"
            "-outer\n");
}

    TEST_CASE("scope-timestamp-format", "[scope]") {
        /* with time_enabled, a banner line is prefixed by a UTC time-of-day
         * "HH:MM:SS.mmm " (13 chars).  Value is wall-clock (nondeterministic),
         * so we check the FORMAT/width only.
         */
        stringstream ss;
        FlatSink sink(ss);
        ThreadLogState::log_set_sink(&sink);

        scope_config::time_enabled = true;
        { scope s("foo"); }
        scope_config::time_enabled = false;    /* reset global BEFORE asserting */
        ThreadLogState::log_set_sink(nullptr);

        std::string out = ss.str();

        /* "HH:MM:SS.mmm +foo\n..." */
        REQUIRE(out.size() >= 17);
        auto dig = [&](std::size_t i) { return std::isdigit((unsigned char)out[i]) != 0; };
        REQUIRE(dig(0)); REQUIRE(dig(1)); REQUIRE(out[2] == ':');
        REQUIRE(dig(3)); REQUIRE(dig(4)); REQUIRE(out[5] == ':');
        REQUIRE(dig(6)); REQUIRE(dig(7)); REQUIRE(out[8] == '.');
        REQUIRE(dig(9)); REQUIRE(dig(10)); REQUIRE(dig(11));
        REQUIRE(out[12] == ' ');
        REQUIRE(out.substr(13, 4) == "+foo");
    }

    TEST_CASE("scope-location-inline", "[scope]") {
        /* FlatSink reports no column, so the code location falls back to inline
         * placement: one space, then "[<basename>:<line>]".  (basename:line
         * layout is deterministic; the line value itself is not asserted.)
         */
        stringstream ss;
        FlatSink sink(ss);
        ThreadLogState::log_set_sink(&sink);

        scope_config::location_enabled = true;
        { XO_SCOPE_(s, always); }   /* 'always' so it logs regardless of min_log_level */
        scope_config::location_enabled = false;   /* reset global BEFORE asserting */
        ThreadLogState::log_set_sink(nullptr);

        std::string out = ss.str();
        auto b = out.find("[scope.test.cpp:");
        REQUIRE(b != std::string::npos);
        REQUIRE(b >= 1);
        REQUIRE(out[b - 1] == ' ');   /* inline: single space precedes '[' */
    }

} /*namespace ut*/

/* end scope.test.cpp */
