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

    int
    fib(int n) {
        XO_SCOPE(log);

        int retval = 1;

        if (n >= 2) {
            log(":n ", n);
            retval = fib(n - 1) + fib(n - 2);
        }

        log(":n ", n, " -> :retval ", retval);

        return retval;
    }

    int
    main(int argc, char ** argv) {
      XO_SCOPE(log);

       int n = 4;
      int fn = fib(n);

      log(":n ", n, " :fib(n) ", fn);
    }

output:

    +main
     +fib
      :n 4
      +fib
       :n 3
       +fib
        :n 2
        +fib
         :n 1 -> :retval 1
        -fib
        +fib
         :n 0 -> :retval 1
        -fib
        :n 2 -> :retval 2
       -fib
       +fib
        :n 1 -> :retval 1
       -fib
       :n 3 -> :retval 3
      -fib
      +fib
       :n 2
       +fib
        :n 1 -> :retval 1
       -fib
       +fib
        :n 0 -> :retval 1
       -fib
       :n 2 -> :retval 2
      -fib
      :n 4 -> :retval 5
     -fib
     :n 4 :fib(n) 5
    -main
