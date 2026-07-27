/* example ex3a/ex3a.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * xo-ppsink port of xo-indentlog/example/ex3, slice A: tag / xtag.
 *
 * Same recursion + level-gating + end_scope as ex2, but log arguments are
 * now key/value tags (tag("n", n), xtag("retval", retval)) instead of ad-hoc
 * ":n " literals.  Color, timestamps, function-name styling and code
 * location (the rest of ex3) arrive in later slices ex3b..ex3d.
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
    scope log(XO_ENTER0(info), tag("n", n));

    int retval = 1;

    if (n >= 2) {
        retval = fib(n - 1) + fib(n - 2);
    }

    log.end_scope(tag("n", n), " <-", xtag("retval", retval));

    return retval;
}

int
main(int argc, char ** argv) {
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 4;

    int n = 3;

    scope log(XO_ENTER0(info), tag("n", n));

    int fn = fib(n);

    log && log(tag("n", n));
    log && log("<-", xtag("fib(n)", fn));
}

/* end ex3a.cpp */
