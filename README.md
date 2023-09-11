# nestlog -- logging with automatic indenting according to call graph

Nestlog is a lightweight header-only library for console logging.

## Examples

    /* examples/ex1/ex1.cpp */

    #include "nestlog/scope.hpp"

    using namespace xo;

    void A(int x) {
        XO_SCOPE(log)  // i.e. xo::scope log("A");

        log(":x ", x);
    }

    int
    main(int argc, char ** argv) {
        A(66);
    }

output:

    +A
     :x 66
    -A
