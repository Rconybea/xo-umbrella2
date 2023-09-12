/* examples ex3/ex3.cpp */

#include "nestlog/scope.hpp"

using namespace xo;

int
fib(int n) {
    scope log(XO_SSETUP0(), ":n ", n);

    int retval = 1;

    if (n >= 2) {
        retval = fib(n - 1) + fib(n - 2);
        log(":n ", n);
    }

    log.end_scope("<- :retval ", retval);

    return retval;
}

int
main(int argc, char ** argv) {
    log_config::style = FS_Pretty;
    log_config::indent_width = 4;

    int n = 4;

    scope log(XO_SSETUP0(), ":n ", 4);

    int fn = fib(n);

    log(":n ", n);
    log("<- :fib(n) ", fn);
}
