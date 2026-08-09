/* example ex3g/ex3g.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * xo-ppsink port of xo-indentlog/example/ex3, slice E: code location.
 *
 * scope_config::location_enabled appends the scope's source "[file:line]"
 * (captured by XO_ENTER0_).  FlatSink doesn't track a column, so it can't
 * right-align -- it falls back to placing the location inline (one space
 * after the banner).  See xo-indentlog2/example/ex3g for the right-aligned
 * PrettySink version.
 */

#include <xo/ppsink/color.hpp>
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

    if (n >= 2)
        retval = fib(n - 1) + fib(n - 2);

    log.end_scope(tag("n", n), " <-", xtag("retval", retval));

    return retval;
}

int
main(int argc, char ** argv) {
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 2;
    scope_config::location_enabled = true;   /* append [file:line] (inline, FlatSink) */

    fib(3);

    return 0;
}

/* end ex3g.cpp */
