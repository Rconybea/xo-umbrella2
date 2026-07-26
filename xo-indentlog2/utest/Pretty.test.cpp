/** @file Pretty.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include <xo/ppsink/Pretty.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    /** a test type that opts in to structured pretty-printing **/
    struct Point_Pretty { int x_; int y_; };
}

/* Pretty specialization for ut::Point_Pretty.
 * Structure: "(" x "," <split> y ")".  The x/y members are int -- they have
 * no Pretty, so pp_write() renders them via operator<< (atomic leaves).
 */
namespace xo::print {
    template <>
    struct Pretty<ut::Point_Pretty> {
        static void print(PpSink & sink, const ut::Point_Pretty & p) {
            sink.put("(");
            sink.begin();
            pp_write(sink, p.x_);
            sink.put(",");
            sink.split();
            pp_write(sink, p.y_);
            sink.end();
            sink.put(")");
        }
    };
} /*namespace xo::print*/

namespace ut {
    using xo::print::has_pretty;
    using xo::print::pp_write;
    using xo::print::PpSink;
    using xo::print::FlatSink;
    using xo::print::PrettySink;
    using xo::print::PpConfig;
    using xo::mm::ArenaConfig;

    /* compile-time: the concept distinguishes opted-in from fallback types */
    static_assert(has_pretty<Point_Pretty>);
    static_assert(!has_pretty<int>);
    static_assert(!has_pretty<double>);
    static_assert(!has_pretty<std::string>);

    /** render @p fn's output through a FlatSink **/
    template <typename Fn>
    static std::string flat_of(Fn && fn) {
        std::ostringstream ss;
        FlatSink sink(ss);
        fn(sink);
        return ss.str();
    }

    /** render @p fn's output through a PrettySink.
     *  @p margin 0 => use the default soft right margin.
     **/
    template <typename Fn>
    static std::string pretty_of(std::uint32_t margin, Fn && fn) {
        ArenaConfig logbuf_cfg { .name_ = "utest.Pretty", .size_ = 64*1024 };
        PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg);
        if (margin > 0)
            cfg = cfg.with_soft_right_margin(margin);

        PrettySink pp(cfg);
        fn(pp);
        return std::string(pp.output());
    }

    TEST_CASE("pp_write.fallback", "[Pretty]")
    {
        /* types without a Pretty route through operator<< */
        REQUIRE(flat_of([](PpSink & s) { pp_write(s, 42); }) == "42");
        REQUIRE(flat_of([](PpSink & s) { pp_write(s, std::string("hi")); }) == "hi");
        REQUIRE(flat_of([](PpSink & s) { pp_write(s, -7); }) == "-7");

        REQUIRE(pretty_of(0, [](PpSink & s) { pp_write(s, 42); }) == "42");
    }

    TEST_CASE("pp_write.pretty", "[Pretty]")
    {
        Point_Pretty p{3, 4};

        /* Pretty<Point_Pretty> drives the structure; int members fall back */
        REQUIRE(flat_of([&](PpSink & s) { pp_write(s, p); }) == "(3,4)");

        /* wide margin: group fits, split collapses -> same as flat */
        REQUIRE(pretty_of(0, [&](PpSink & s) { pp_write(s, p); }) == "(3,4)");

        /* narrow margin: group doesn't fit -> split becomes newline + indent
         * (indent_width default 2, nesting depth 1 => 2 spaces)
         */
        REQUIRE(pretty_of(2, [&](PpSink & s) { pp_write(s, p); }) == "(3,\n  4)");
    }
}

/* end Pretty.test.cpp */
