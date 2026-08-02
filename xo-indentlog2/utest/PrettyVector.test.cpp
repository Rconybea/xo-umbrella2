/** @file PrettyVector.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Covers Prettifier<std::vector<T>> and Prettifier<FunctionStyle>
 *  (and their composition: a vector of FunctionStyle).
 **/

#include "print/PrettyVector.hpp"
#include "print/PrettyFunctionStyle.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace ut {
    using xo::pp::has_prettifier;
    using xo::pp::pretty;
    using xo::pp::PpSink;
    using xo::pp::FlatSink;
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::FunctionStyle;
    using xo::mm::ArenaConfig;
    using std::vector;

    /* both specializations opt in; element types without one fall back */
    static_assert(has_prettifier<FunctionStyle>);
    static_assert(has_prettifier<vector<int>>);
    static_assert(has_prettifier<vector<FunctionStyle>>);
    static_assert(has_prettifier<vector<vector<int>>>);

    template <typename Fn>
    static std::string flat_of(Fn && fn) {
        std::ostringstream ss;
        FlatSink sink(ss);
        fn(sink);
        return ss.str();
    }

    template <typename Fn>
    static std::string
    pretty_of(std::uint32_t margin, Fn && fn) {
        ArenaConfig logbuf_cfg { .name_ = "utest.PrettyVector", .size_ = 64*1024 };
        PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg);
        if (margin > 0)
            cfg = cfg.with_soft_right_margin(margin);

        PrettySink pp(cfg, nullptr /*out*/);
        fn(pp);
        return std::string(pp.output());
    }

    TEST_CASE("Pretty.FunctionStyle", "[Pretty][FunctionStyle]")
    {
        REQUIRE(flat_of([](PpSink & s) { pretty(s, FunctionStyle::literal); }) == "literal");
        REQUIRE(flat_of([](PpSink & s) { pretty(s, FunctionStyle::pretty); }) == "pretty");
        REQUIRE(flat_of([](PpSink & s) { pretty(s, FunctionStyle::streamlined); }) == "streamlined");
        REQUIRE(flat_of([](PpSink & s) { pretty(s, FunctionStyle::simple); }) == "simple");

        /* atomic: same through the pretty sink */
        REQUIRE(pretty_of(0, [](PpSink & s) { pretty(s, FunctionStyle::simple); }) == "simple");
    }

    TEST_CASE("Pretty.vector.int", "[Pretty][PrettyVector]")
    {
        vector<int> v = {1, 2, 3};

        REQUIRE(flat_of([&](PpSink & s) { pretty(s, v); }) == "[1,2,3]");

        /* wide margin: fits, splits collapse -> same as flat */
        REQUIRE(pretty_of(0, [&](PpSink & s) { pretty(s, v); }) == "[1,2,3]");

        /* narrow margin: doesn't fit -> one element per line, indent 2 */
        REQUIRE(pretty_of(2, [&](PpSink & s) { pretty(s, v); }) == "[1,\n  2,\n  3]");
    }

    TEST_CASE("Pretty.vector.edges", "[Pretty][PrettyVector]")
    {
        REQUIRE(flat_of([](PpSink & s) { pretty(s, vector<int>{}); }) == "[]");
        REQUIRE(flat_of([](PpSink & s) { pretty(s, vector<int>{42}); }) == "[42]");

        /* empty vector: empty group must not block, no split */
        REQUIRE(pretty_of(1, [](PpSink & s) { pretty(s, vector<int>{}); }) == "[]");
    }

    TEST_CASE("Pretty.vector.nested", "[Pretty][PrettyVector]")
    {
        vector<vector<int>> v = {{1, 2}, {3}};

        /* inner Prettifier<vector<int>> is reached via pretty on each element */
        REQUIRE(flat_of([&](PpSink & s) { pretty(s, v); }) == "[[1,2],[3]]");
        REQUIRE(pretty_of(0, [&](PpSink & s) { pretty(s, v); }) == "[[1,2],[3]]");
    }

    TEST_CASE("Pretty.vector.of_functionstyle", "[Pretty][PrettyVector][FunctionStyle]")
    {
        vector<FunctionStyle> v = {FunctionStyle::literal, FunctionStyle::simple};

        /* composition: vector printer + FunctionStyle printer */
        REQUIRE(flat_of([&](PpSink & s) { pretty(s, v); }) == "[literal,simple]");
    }
}

/* end PrettyVector.test.cpp */
