# nestlog -- logging with automatic indenting according to call graph

Nestlog is a lightweight header-only library for console logging.

## Examples

### 1

    /* examples/ex1/ex1.cpp */

    #include "nestlog/scope.hpp"

    void A(int x) {
        XO_SCOPE(log);  // i.e. xo::scope log("A");

        log("enter ", ":x ", x);
    }

    int
    main(int argc, char ** argv) {
        A(66);
    }

output:

    +A
     enter :x 66
    -A

### 2

    /* examples ex2/ex2.cpp */

    #include "nestlog/scope.hpp"

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

output:

    20:13:12.992909 +(0) main :n 4                                   [ex2.cpp:30]
    20:13:12.992968     +(1) fib :n 4                                [ex2.cpp:9]
    20:13:12.992986         +(2) fib :n 3                            [ex2.cpp:9]
    20:13:12.992999             +(3) fib :n 2                        [ex2.cpp:9]
    20:13:12.993002                 +(4) fib :n 1                    [ex2.cpp:9]
    20:13:12.993012                 -(4) fib <- :retval 1
    20:13:12.993022                 +(4) fib :n 0                    [ex2.cpp:9]
    20:13:12.993032                 -(4) fib <- :retval 1
                                    :n 2
    20:13:12.993049             -(3) fib <- :retval 2
    20:13:12.993059             +(3) fib :n 1                        [ex2.cpp:9]
    20:13:12.993069             -(3) fib <- :retval 1
                                :n 3
    20:13:12.993085         -(2) fib <- :retval 3
    20:13:12.993095         +(2) fib :n 2                            [ex2.cpp:9]
    20:13:12.993105             +(3) fib :n 1                        [ex2.cpp:9]
    20:13:12.993115             -(3) fib <- :retval 1
    20:13:12.993124             +(3) fib :n 0                        [ex2.cpp:9]
    20:13:12.993134             -(3) fib <- :retval 1
                                :n 2
    20:13:12.993145         -(2) fib <- :retval 2
                            :n 4
    20:13:12.993155     -(1) fib <- :retval 5
                        :n 4
                        <- :fib(n) 5
    20:13:12.993172 -(0) main
