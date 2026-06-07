/* @file ex_qty.cpp */

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"

using namespace std;

int
main() {
    using namespace xo::qty;
    namespace u = xo::qty::u;

    static_assert(u::meter.n_bpu() == 1);

    //constexpr auto q = qty::meters(2) + u::meter;
}

/* end ex_qty.cpp */
