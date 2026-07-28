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

    TEST_CASE("scope-indent-flat", "[scope]")
    {

        scope_config::time_enabled = false;

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
                "+(0) outer\n"
                "  hello\n"
                "  +(1) inner\n"
                "    world42\n"
                "  -(1) inner\n"
                "  bye\n"
                "-(0) outer\n");
    }

    TEST_CASE("scope-timestamp-format", "[scope]")
    {
        /* with time_enabled, a banner line is prefixed by a UTC time-of-day
         * "HH:MM:SS.uuuuuu " (16 chars).  Value is wall-clock (nondeterministic),
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

        /* "HH:MM:SS.uuuuuu +foo\n..." */
        REQUIRE(out.size() >= 20);
        auto dig = [&](std::size_t i) { return std::isdigit((unsigned char)out[i]) != 0; };
        REQUIRE(dig(0)); REQUIRE(dig(1)); REQUIRE(out[2] == ':');
        REQUIRE(dig(3)); REQUIRE(dig(4)); REQUIRE(out[5] == ':');
        REQUIRE(dig(6)); REQUIRE(dig(7)); REQUIRE(out[8] == '.');
        REQUIRE(dig(9)); REQUIRE(dig(10)); REQUIRE(dig(11));
        REQUIRE(dig(12)); REQUIRE(dig(13)); REQUIRE(dig(14));
        REQUIRE(out[15] == ' ');
        REQUIRE(out.substr(16, 8) == "+(0) foo");
    }

    TEST_CASE("scope-location-inline", "[scope]")
    {
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

    TEST_CASE("scope-retroactively-enable", "[scope]") {
        /* a scope created below the log threshold stays silent, but
         * retroactively_enable() turns it on and emits the deferred banner.
         */
        bool saved_time = scope_config::time_enabled;
        scope_config::time_enabled = false;   /* deterministic output */

        stringstream ss;
        FlatSink sink(ss);
        ThreadLogState::log_set_sink(&sink);

        std::string before;
        {
            /* min_log_level defaults to error; 'chatty' < error => disabled */
            scope log(XO_ENTER0_(chatty), "ctor-args");
            before = ss.str();
            log.retroactively_enable("late", 42);
        }

        scope_config::time_enabled = saved_time;   /* reset BEFORE asserting */
        ThreadLogState::log_set_sink(nullptr);

        std::string out = ss.str();
        REQUIRE(before.empty());                             /* disabled: nothing logged */
        REQUIRE(out.find("late42") != std::string::npos);    /* retro banner emitted */
        REQUIRE(out.find("ctor-args") == std::string::npos); /* original ctor args discarded */
    }

} /*namespace ut*/

/* end scope.test.cpp */
