/* example ex2/ex2.cpp
 *
 * xo-ppsink port of xo-indentlog/example/ex2, using xo::pp::scope.
 *
 * Demonstrates recursion, log-level gating (the `log && log(...)` idiom),
 * and an explicit end_scope() that reports the return value.
 */

#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>

using xo::pp::scope;
using xo::pp::scope_config;
using xo::pp::log_level;

int
fib(int n) {
    scope log(XO_ENTER0_(info), ":n ", n);

    int retval = 1;

    if (n >= 2) {
        retval = fib(n - 1) + fib(n - 2);
        log && log(":n ", n);
    }

    log.end_scope("<- :retval ", retval);

    return retval;
}

int
main(int argc, char ** argv) {
    scope_config::min_log_level = log_level::info;
    scope_config::indent_width = 4;

    int n = 4;

    scope log(XO_ENTER0_(info), ":n ", n);

    int fn = fib(n);

    log && log(":n ", n);
    log && log("<- :fib(n) ", fn);
}

/* end ex2.cpp */
