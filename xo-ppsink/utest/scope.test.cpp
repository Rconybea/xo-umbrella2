/** @file scope.test.cpp **/

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/color.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <catch2/catch.hpp>
#include <cctype>
#include <sstream>
#include <memory>

namespace ut {

    using xo::pp::scope;
    using xo::pp::scope_config;
    using xo::pp::FlatSink;
    using xo::pp::PpStyle;
    using xo::pp::ThreadLogState;
    using std::stringstream;

    /* The gate's DEFAULT, asserted directly.  Every other test in this file
     * turns color off to pin text, so without this one the default could be
     * flipped back to false and the suite would stay green -- which is how it
     * came to be false in the first place (color.hpp).
     */
    TEST_CASE("color-enabled-by-default", "[scope][color]")
    {
        scope_config::time_enabled = false;

        stringstream ss;
        auto sink = std::make_unique<FlatSink>(PpStyle::colored(), ss.rdbuf());
        ThreadLogState::log_set_sink(std::move(sink));
        {
            scope outer("outer");
        }
        ThreadLogState::log_set_sink(nullptr);

        /* the "(N)" nesting level is colored (scope_config::nesting_level_color,
         * xterm 153), so the banner carries SGR escapes
         */
        REQUIRE(ss.str().find('\033') != std::string::npos);

        /* ... and with the gate off, byte-identical text without them */
        {
            stringstream plain_ss;
            auto plain_sink = std::make_unique<FlatSink>(plain_ss.rdbuf());

            ThreadLogState::log_set_sink(std::move(plain_sink));
            {
                scope outer("outer");
            }
            ThreadLogState::log_set_sink(nullptr);

            REQUIRE(plain_ss.str() == "+(0) outer\n-(0) outer\n");
        }
    }

    TEST_CASE("scope-indent-flat", "[scope]")
    {
        scope_config::time_enabled = false;

        /* capture scope output into a stringstream via a FlatSink */
        stringstream ss;
        auto sink = std::make_unique<FlatSink>(ss.rdbuf());

        ThreadLogState::log_set_sink(std::move(sink));
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
        auto sink = std::make_unique<FlatSink>(ss.rdbuf());
        ThreadLogState::log_set_sink(std::move(sink));

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
        auto sink = std::make_unique<FlatSink>(ss.rdbuf());
        ThreadLogState::log_set_sink(std::move(sink));

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
        auto sink = std::make_unique<FlatSink>(ss.rdbuf());
        ThreadLogState::log_set_sink(std::move(sink));

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

    TEST_CASE("scope-xo-debug-gating", "[scope]") {
        /* XO_DEBUG_(flag) enables logging (at 'always') iff flag is true. */
        bool saved_time = scope_config::time_enabled;
        scope_config::time_enabled = false;   /* deterministic output */

        auto run = [](bool flag) {
            stringstream ss;
            auto sink = std::make_unique<FlatSink>(ss.rdbuf());
            ThreadLogState::log_set_sink(std::move(sink));
            {
                scope log(XO_DEBUG_(flag));
                log && log("banner", flag);
            }
            ThreadLogState::log_set_sink(nullptr);
            return ss.str();
        };

        std::string on = run(true);
        std::string off = run(false);

        scope_config::time_enabled = saved_time;   /* reset BEFORE asserting */

        REQUIRE(on.find("banner") != std::string::npos);   /* flag=true => enabled */
        REQUIRE(off.empty());                              /* flag=false => never => silent */
    }

    /* XO_LITERAL_ banners from two runtime names, mirroring legacy XO_LITERAL:
     * name1 is styled/colored, name2 appended verbatim with no separator (so
     * the caller supplies its own "::"), e.g. self_type + "::ctor".
     */
    TEST_CASE("scope-literal-two-names", "[scope]")
    {
        bool saved_time = scope_config::time_enabled;
        scope_config::time_enabled = false;

        stringstream ss;
        auto sink = std::make_unique<FlatSink>(ss.rdbuf());

        ThreadLogState::log_set_sink(std::move(sink));
        {
            scope lscope(XO_LITERAL_(always, "Refcounted", "::ctor"));
        }
        ThreadLogState::log_set_sink(nullptr);

        scope_config::time_enabled = saved_time;   /* reset BEFORE asserting */

        REQUIRE(ss.str() ==
                "+(0) Refcounted::ctor\n"
                "-(0) Refcounted::ctor\n");
    }

    /* an omitted second name must change nothing: every existing call site
     * builds a scope_setup without one.
     */
    TEST_CASE("scope-literal-one-name-unchanged", "[scope]")
    {
        bool saved_time = scope_config::time_enabled;
        scope_config::time_enabled = false;

        stringstream ss;
        auto sink = std::make_unique<FlatSink>(ss.rdbuf());

        ThreadLogState::log_set_sink(std::move(sink));
        {
            scope lscope(XO_ENTER2_(always, true, "Refcounted"));
        }
        ThreadLogState::log_set_sink(nullptr);

        scope_config::time_enabled = saved_time;

        REQUIRE(ss.str() ==
                "+(0) Refcounted\n"
                "-(0) Refcounted\n");
    }

} /*namespace ut*/

/* end scope.test.cpp */
