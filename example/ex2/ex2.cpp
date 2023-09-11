/* examples ex2/ex2.cpp */

#include "nestlog/scope.hpp"

int
fib(int n) {
    XO_SCOPE(log);

    int retval = 1;

    if (n >= 2)
        retval = fib(n - 1) + fib(n - 2);

    log("result ", ":retval ", retval);

    return retval;
}

int
main(int argc, char ** argv) {
    XO_SCOPE(log);

    int n = 4;
    int fn = fib(n);

    log(":n ", n, " :fib(n) ", fn);
}
