/* example ex3g/ex3g.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * Code location, RIGHT-ALIGNED, through the arena-backed PrettySink.
 *
 * PrettySink tracks the visible output column (PpSink::lpos), so the scope
 * logger right-aligns "[file:line]" at scope_config::location_tab -- the
 * locations line up in a column regardless of nesting depth.  (FlatSink, which
 * has no column, falls back to inline placement -- see xo-ppsink/example/ex3g.)
 */

#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <xo/ppsink/color.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <iostream>
#include <string>

using xo::pp::scope;
using xo::pp::scope_config;
using xo::pp::log_level;
using xo::pp::tag;
using xo::pp::xtag;
using xo::pp::PrettySink;
using xo::pp::PpConfig;
using xo::pp::ThreadLogState;
using xo::mm::ArenaConfig;

int
fib(int n) {
    scope log(XO_ENTER0_(info), tag("n", n));

    int retval = 1;

    if (n >= 2)
        retval = fib(n - 1) + fib(n - 2);

    log.end_scope(tag("n", n), " <-", xtag("retval", retval));

    return retval;
}

int
main(int argc, char ** argv) {
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 2;
    scope_config::location_enabled = true;   /* append [file:line] */
    scope_config::location_tab = 40;         /* right-align it at column 40 */

    ArenaConfig logbuf_cfg { .name_ = "example.ex3g", .size_ = 64*1024 };
    PrettySink pp(PpConfig().with_logbuf_config(logbuf_cfg),
                  nullptr /*out*/);

    ThreadLogState::log_set_sink(&pp);
    fib(3);
    ThreadLogState::log_set_sink(nullptr);   /* restore default (clog) */

    std::cout << pp.output();

    return 0;
}

/* end ex3g.cpp */
