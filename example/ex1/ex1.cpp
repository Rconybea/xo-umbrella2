#include "indentlog/scope.hpp"

using namespace xo;

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
