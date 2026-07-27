/* example ex3f/ex3f.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * Colored nesting counters, routed through the arena-backed PrettySink
 * (from xo-indentlog2) -- like xo-ppsink/example/ex3f but exercising the real
 * pretty-print engine rather than the degenerate FlatSink.
 *
 * scope_config::nesting_level_enabled shows the depth "(N)" after the +/-
 * banner marker; nesting_level_color colors just the depth (xterm 195).
 * Run in a terminal to see color; piped, the raw \033[...m escapes show.
 */

#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/color.hpp>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <iostream>
#include <string>

/* XO_ENTER0_ (trailing underscore) coexists with legacy xo-indentlog's
 * XO_ENTER0, which PrettySink -> xo-arena pulls into this TU.
 */

using xo::pp::scope;
using xo::pp::scope_config;
using xo::pp::color_config;
using xo::pp::color_spec_type;
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
    color_config::color_enabled = true;
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 2;
    scope_config::nesting_level_enabled = true;                    /* show "(N)" depth */
    scope_config::nesting_level_color = color_spec_type::xterm(153);  /* light blue (legacy default xterm 195 is near-white/pale) */

    ArenaConfig logbuf_cfg { .name_ = "example.ex3f", .size_ = 64*1024 };
    PrettySink pp(PpConfig().with_logbuf_config(logbuf_cfg));

    ThreadLogState::log_set_sink(&pp);
    fib(3);
    ThreadLogState::log_set_sink(nullptr);   /* restore default (clog) */

    std::cout << pp.output();

    return 0;
}

/* end ex3f.cpp */
