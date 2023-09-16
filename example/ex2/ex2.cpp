/* examples ex2/ex2.cpp */

#include "indentlog/scope.hpp"

using namespace xo;

int
fib(int n) {
    scope log(XO_ENTER0(info), ":n ", n);

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
    log_config::min_log_level = xo::log_level::info;
    log_config::indent_width = 4;

    int n = 4;

    scope log(XO_ENTER0(info), ":n ", 4);

    int fn = fib(n);

    log && log(":n ", n);
    log && log("<- :fib(n) ", fn);
}
