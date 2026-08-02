/** @file Prettifier.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty_ostream.hpp> /* Fallback_int exercises the operator<< fallback */
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    /* A leaf type OWNED by this TU: no Prettifier specialization, plus an
     * operator<<, so pretty() exercises the operator<< fallback path.
     *
     * Deliberately a bespoke wrapper rather than a built-in scalar (int,
     * double, ...): ppsink is free to add Prettifier<int> and friends, which
     * would silently turn a built-in fallback example into a Prettifier type and
     * break this test.  A type we own can't be reclassified out from under us.
     */
    struct Fallback_int { int v_; };

    inline std::ostream &
    operator<<(std::ostream & os, const Fallback_int & x) {
        return os << x.v_;
    }

    /** a test type that opts in to structured pretty-printing **/
    struct Point_Pretty { Fallback_int x_; Fallback_int y_; };
}

/* Prettifier specialization for ut::Point_Pretty.
 * Structure: "(" x "," <split> y ")".  The x/y members are Fallback_int --
 * they have no Prettifier, so pretty() renders them via operator<< (atomic
 * leaves), exercising a Prettifier type that delegates to fallback members.
 */
namespace xo::pp {
    template <>
    struct Prettifier<ut::Point_Pretty> {
        static void print(PpSink & sink, const ut::Point_Pretty & p) {
            sink.put("(");
            sink.begin();
            pretty(sink, p.x_);
            sink.put(",");
            sink.split();
            pretty(sink, p.y_);
            sink.end();
            sink.put(")");
        }
    };
} /*namespace xo::pp*/

namespace ut {
    using xo::pp::has_prettifier;
    using xo::pp::pretty;
    using xo::pp::PpSink;
    using xo::pp::FlatSink;
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::mm::ArenaConfig;

    /* compile-time: the concept distinguishes opted-in from fallback types.
     * Assert only on types this TU owns -- whether ppsink gives int/double/
     * std::string a Prettifier is ppsink's business, tested in ppsink's own suite.
     */
    static_assert(has_prettifier<Point_Pretty>);
    static_assert(!has_prettifier<Fallback_int>);

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

        PrettySink pp(cfg, nullptr);
        fn(pp);
        return std::string(pp.output());
    }

    TEST_CASE("pretty.fallback", "[Pretty]")
    {
        /* a type with neither Pretty nor string-conversion routes through
         * operator<< */
        REQUIRE(flat_of([](PpSink & s) { pretty(s, Fallback_int{42}); }) == "42");
        REQUIRE(flat_of([](PpSink & s) { pretty(s, Fallback_int{-7}); }) == "-7");

        REQUIRE(pretty_of(0, [](PpSink & s) { pretty(s, Fallback_int{42}); }) == "42");
    }

    TEST_CASE("pretty.pretty", "[Pretty]")
    {
        Point_Pretty p{ {3}, {4} };

        /* Prettifier<Point_Pretty> drives the structure; int members fall back */
        REQUIRE(flat_of([&](PpSink & s) { pretty(s, p); }) == "(3,4)");

        /* wide margin: group fits, split collapses -> same as flat */
        REQUIRE(pretty_of(0, [&](PpSink & s) { pretty(s, p); }) == "(3,4)");

        /* narrow margin: group doesn't fit -> split becomes newline + indent
         * (indent_width default 2, nesting depth 1 => 2 spaces)
         */
        REQUIRE(pretty_of(2, [&](PpSink & s) { pretty(s, p); }) == "(3,\n  4)");
    }
}

/* end Prettifier.test.cpp */
