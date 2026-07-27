/** @file scope.test.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Verify the ppsink <-> indentlog2 seam: the arena-free xo::scope (from
 *  xo-ppsink) drives an arena-backed PrettySink (from xo-indentlog2) exactly
 *  as it drives a FlatSink, when the PrettySink is installed as the thread's
 *  active sink.
 **/

#include <xo/ppsink/scope.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <string>

namespace ut {
    using xo::pp::scope;
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::ThreadLogState;
    using xo::mm::ArenaConfig;

    /** run @p fn with a PrettySink installed as the thread's active sink;
     *  return the pretty-printed output.
     **/
    template <typename Fn>
    static std::string scoped_pretty(std::uint32_t margin, Fn && fn) {
        ArenaConfig logbuf_cfg { .name_ = "utest.scope", .size_ = 64*1024 };
        PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg);
        if (margin > 0)
            cfg = cfg.with_soft_right_margin(margin);

        PrettySink pp(cfg);
        ThreadLogState::log_set_sink(&pp);
        fn();
        ThreadLogState::log_set_sink(nullptr);   /* restore default (clog) */
        return std::string(pp.output());
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

    TEST_CASE("scope-over-prettysink", "[scope][prettysink]") {
        /* wide (default) margin: nothing breaks, so pretty output matches the
         * flat output produced by the same scope code in xo-ppsink's test.
         */
        REQUIRE(scoped_pretty(0, nested_scopes) ==
                "+outer\n"
                "  hello\n"
                "  +inner\n"
                "    world42\n"
                "  -inner\n"
                "  bye\n"
                "-outer\n");
    }
}

/* end scope.test.cpp */
