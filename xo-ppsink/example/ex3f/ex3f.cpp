/* example ex3f/ex3f.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * Nesting-level display (parity with xo-indentlog/example/ex4).
 *
 * With scope_config::nesting_level_enabled, each entry/exit banner shows the
 * nesting depth as "(N)" right after the +/- marker.  Here the depth is
 * colored (nesting_level_color, xterm 195) while the rest is left plain, to
 * isolate the feature.
 */

#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/color.hpp>

using xo::pp::scope;
using xo::pp::scope_config;
using xo::pp::color_config;
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
    scope_config::nesting_level_enabled = true;   /* show "(N)" depth on banners */
    color_config::color_enabled = true;           /* color the "(N)" depth (default light-blue) */

    fib(3);

    return 0;
}

/* end ex3f.cpp */
