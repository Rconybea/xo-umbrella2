/** @file scope.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Verify the ppsink <-> indentlog2 seam: the arena-free xo::scope (from
 *  xo-ppsink) drives an arena-backed PrettySink (from xo-indentlog2) exactly
 *  as it drives a FlatSink, when the PrettySink is installed as the thread's
 *  active sink.
 **/

#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <xo/ppsink/color.hpp>
#include <xo/ppsink/scope.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    using xo::pp::scope;
    using xo::pp::scope_config;
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::PpStyle;
    using xo::pp::ThreadLogState;
    using xo::mm::ArenaConfig;

    /** run @p fn with a PrettySink installed as the thread's active sink;
     *  return the pretty-printed output.
     *
     *  Capture via a dest streambuf (not pp.output()): PrettySink drains each
     *  completed record to the attached sink and reclaims its buffer, so
     *  output() only ever holds the current (unfinished) record.
     **/
    template <typename Fn>
    static std::string scoped_pretty(std::uint32_t margin, Fn && fn) {
        ArenaConfig logbuf_cfg { .name_ = "utest.scope", .size_ = 64*1024 };
        PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg).with_style(PpStyle::plain());
        if (margin > 0)
            cfg = cfg.with_soft_right_margin(margin);

        std::ostringstream oss;
        PrettySink pp(cfg, oss.rdbuf());

        ThreadLogState::log_set_sink(&pp);
        fn();
        ThreadLogState::log_set_sink(nullptr);   /* restore default (clog) */

        return oss.str();
    }

    /** the same nested-scope sequence exercised by xo-ppsink's FlatSink test **/
    static void nested_scopes() {
        scope outer("outer");
        outer.log("hello");
        {
            scope inner("inner");
            inner.log("world", 42);
        }
        outer.log("bye");
    }

    TEST_CASE("scope-over-prettysink", "[scope][prettysink]")
    {
        scope_config::time_enabled = false;

        /* wide (default) margin: nothing breaks, so pretty output matches the
         * flat output produced by the same scope code in xo-ppsink's test.
         */
        REQUIRE(scoped_pretty(0, nested_scopes) ==
                "+(0) outer\n"
                "  hello\n"
                "  +(1) inner\n"
                "    world42\n"
                "  -(1) inner\n"
                "  bye\n"
                "-(0) outer\n");
    }
}

/* end scope.test.cpp */
