/* ex1.cpp
 *
 * xo-ppsink port of xo-indentlog/example/ex1, using xo::pp::scope.
 *
 * Same nested-scope structure as the legacy example, but the scope logger
 * here is arena-free: output goes through the thread's active PpSink
 * (default: a FlatSink over std::clog).
 */

#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>

using xo::pp::scope;

void inner(int x) {
    scope log(XO_ENTER0(always), ":x ", x);
}

void outer(int y) {
    scope log(XO_ENTER0(always), ":y ", y);

    inner(2*y);
}

int
main(int argc, char ** argv) {
    outer(123);
}

/* end ex1.cpp */
