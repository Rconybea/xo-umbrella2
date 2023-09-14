/* examples ex3/ex3.cpp */

#include "nestlog/scope.hpp"

using namespace xo;

int
fib(int n) {
    scope log(XO_ENTER0(), tag("n", n));

    int retval = 1;

    if (n >= 2) {
        retval = fib(n - 1) + fib(n - 2);
        log(tag("n", n));
    }

    log.end_scope("<-", xtag("retval", retval));

    return retval;
}

int
main(int argc, char ** argv) {
    log_config::style = FS_Pretty;
    log_config::indent_width = 4;
    log_config::location_tab = 40;

    int n = 4;

    scope log(XO_ENTER0(), ":n ", 4);

    int fn = fib(n);

    log(xtag("n", n));
    log("<-", xtag("fib(n)", fn));
}

/* ex3/ex3.cpp */
