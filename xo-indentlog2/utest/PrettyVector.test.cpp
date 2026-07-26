/** @file PrettyVector.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Covers Pretty<std::vector<T>> and Pretty<FunctionStyle>
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
    using xo::print::has_pretty;
    using xo::print::pp_write;
    using xo::print::PpSink;
    using xo::print::FlatSink;
    using xo::print::PrettySink;
    using xo::print::PpConfig;
    using xo::FunctionStyle;
    using xo::mm::ArenaConfig;
    using std::vector;

    /* both specializations opt in; element types without one fall back */
    static_assert(has_pretty<FunctionStyle>);
    static_assert(has_pretty<vector<int>>);
    static_assert(has_pretty<vector<FunctionStyle>>);
    static_assert(has_pretty<vector<vector<int>>>);

    template <typename Fn>
    static std::string flat_of(Fn && fn) {
        std::ostringstream ss;
        FlatSink sink(ss);
        fn(sink);
        return ss.str();
    }

    template <typename Fn>
    static std::string pretty_of(std::uint32_t margin, Fn && fn) {
        ArenaConfig logbuf_cfg { .name_ = "utest.PrettyVector", .size_ = 64*1024 };
        PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg);
        if (margin > 0)
            cfg = cfg.with_soft_right_margin(margin);

        PrettySink pp(cfg);
        fn(pp);
        return std::string(pp.output());
    }

    TEST_CASE("Pretty.FunctionStyle", "[Pretty][FunctionStyle]")
    {
        REQUIRE(flat_of([](PpSink & s) { pp_write(s, FunctionStyle::literal); }) == "literal");
        REQUIRE(flat_of([](PpSink & s) { pp_write(s, FunctionStyle::pretty); }) == "pretty");
        REQUIRE(flat_of([](PpSink & s) { pp_write(s, FunctionStyle::streamlined); }) == "streamlined");
        REQUIRE(flat_of([](PpSink & s) { pp_write(s, FunctionStyle::simple); }) == "simple");

        /* atomic: same through the pretty sink */
        REQUIRE(pretty_of(0, [](PpSink & s) { pp_write(s, FunctionStyle::simple); }) == "simple");
    }

    TEST_CASE("Pretty.vector.int", "[Pretty][PrettyVector]")
    {
        vector<int> v = {1, 2, 3};

        REQUIRE(flat_of([&](PpSink & s) { pp_write(s, v); }) == "[1,2,3]");

        /* wide margin: fits, splits collapse -> same as flat */
        REQUIRE(pretty_of(0, [&](PpSink & s) { pp_write(s, v); }) == "[1,2,3]");

        /* narrow margin: doesn't fit -> one element per line, indent 2 */
        REQUIRE(pretty_of(2, [&](PpSink & s) { pp_write(s, v); }) == "[1,\n  2,\n  3]");
    }

    TEST_CASE("Pretty.vector.edges", "[Pretty][PrettyVector]")
    {
        REQUIRE(flat_of([](PpSink & s) { pp_write(s, vector<int>{}); }) == "[]");
        REQUIRE(flat_of([](PpSink & s) { pp_write(s, vector<int>{42}); }) == "[42]");

        /* empty vector: empty group must not block, no split */
        REQUIRE(pretty_of(1, [](PpSink & s) { pp_write(s, vector<int>{}); }) == "[]");
    }

    TEST_CASE("Pretty.vector.nested", "[Pretty][PrettyVector]")
    {
        vector<vector<int>> v = {{1, 2}, {3}};

        /* inner Pretty<vector<int>> is reached via pp_write on each element */
        REQUIRE(flat_of([&](PpSink & s) { pp_write(s, v); }) == "[[1,2],[3]]");
        REQUIRE(pretty_of(0, [&](PpSink & s) { pp_write(s, v); }) == "[[1,2],[3]]");
    }

    TEST_CASE("Pretty.vector.of_functionstyle", "[Pretty][PrettyVector][FunctionStyle]")
    {
        vector<FunctionStyle> v = {FunctionStyle::literal, FunctionStyle::simple};

        /* composition: vector printer + FunctionStyle printer */
        REQUIRE(flat_of([&](PpSink & s) { pp_write(s, v); }) == "[literal,simple]");
    }
}

/* end PrettyVector.test.cpp */
