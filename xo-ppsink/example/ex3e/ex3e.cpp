/* example ex3e/ex3e.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * xo-ppsink port of xo-indentlog/example/ex3, slice D: timestamps.
 *
 * With scope_config::time_enabled, each banner line is prefixed with a UTC
 * time-of-day; mid-scope log() lines get a same-width blank pad so they align
 * under the banner.  (Uses xo-timeutil -- the dep ppsink was forward-
 * provisioned with.)  Timestamps are wall-clock, so output varies per run.
 */

#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>

using xo::pp::scope;
using xo::pp::scope_config;
using xo::pp::log_level;
using xo::pp::tag;
using xo::pp::xtag;

int
fib(int n) {
    scope log(XO_ENTER0_(info), tag("n", n));

    int retval = 1;

    if (n >= 2) {
        retval = fib(n - 1) + fib(n - 2);
        log && log(tag("n", n));   /* mid-scope line: blank time pad, aligned */
    }

    log.end_scope(tag("n", n), " <-", xtag("retval", retval));

    return retval;
}

int
main(int argc, char ** argv) {
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 2;
    scope_config::time_enabled = true;   /* UTC millisecond timestamps */

    fib(3);

    return 0;
}

/* end ex3e.cpp */
