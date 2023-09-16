# nestlog -- logging with automatic call-graph indenting

Nestlog is a lightweight header-only library for console logging.

## Features

- header-only;  nothing to link
- easy-to-read format uses indenting to show call structure.
  indentation has user-controlled upper limit to preserve readability with
  deeply nested call graphs
- colorized output using vt100 color codes (ansi or xterm)
- automatically captures + displays timestamp, function name and code location.
  supports several function-name formats to reflect tradeoff readability for precision
- application code may issue logging code that contains embedded newlines and/or color escapes;
  logger preserves indentation.
- logger is 'truthy' -> only pay for formatting when entry points is enabled.
- also provides family of convenience stream-inserters

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
        log_config::min_log_level = log_level::info;
        log_config::time_enabled = true;
        log_config::time_local_flag = true;
        log_config::style = FS_Streamlined;
        log_config::indent_width = 4;
        log_config::max_indent_width = 30;
        log_config::location_tab = 80;
        log_config::encoding = CE_Xterm;
        log_config::function_entry_color = 69;
        log_config::function_exit_color = 70;
        log_config::code_location_color = 166;

        int n = 3;

        scope log(XO_ENTER0(info), ":n ", 4);

        int fn = fib(n);

        log && log(tag("n", n));
        log && log("<-", xtag("fib(n)", fn));
    }

output:

  [ex3 output](img/ex3.png)

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
