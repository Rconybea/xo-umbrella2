#include "nestlog/scope.hpp"

using namespace xo;

void A(int x) {
    XO_SCOPE(log, info);

    log("x:", x);
}

int
main(int argc, char ** argv) {
    A(66);
}
