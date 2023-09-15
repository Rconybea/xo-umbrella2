/* examples ex3/ex3.cpp */

#include "nestlog/scope.hpp"

using namespace xo;

int
fib(int n) {
    scope log(XO_ENTER0(), tag("n", n));

    int retval = 1;

    if (n >= 2) {
        retval = fib(n - 1) + fib(n - 2);
    }

    log.end_scope(tag("n", n), " <-", xtag("retval", retval));

    return retval;
}

int
main(int argc, char ** argv) {
    log_config::style = FS_Streamlined;
    log_config::indent_width = 4;
    log_config::max_indent_width = 14;
    log_config::location_tab = 70;
    log_config::encoding = CE_Xterm;
    log_config::function_entry_color = 69;
    log_config::function_exit_color = 70;
    log_config::code_location_color = 166;

    int n = 4;

    scope log(XO_ENTER0(), ":n ", 4);

    int fn = fib(n);

    xtag("n", n);
    log("<-", xtag("fib(n)", fn));
}

/* ex3/ex3.cpp */
