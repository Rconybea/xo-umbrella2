/** @file ex8.cpp **/

#include "xo/unit/xquantity.hpp"
#include "xo/unit/xquantity_iostream.hpp"
#include <iostream>

int
main () {
    using namespace xo::qty;
    namespace u = xo::qty::u;
    using namespace std;

    constexpr xquantity qty1(7, u::foot);
    constexpr xquantity qty2(6.0, u::inch);

    // constexpr not supported for xquantity addition
    xquantity qty3 = qty1 + qty2;

    cerr << "qty1: " << qty1 << endl;
    cerr << "qty2: " << qty2 << endl;
    cerr << "qty3: " << qty3 << endl;

    /* rescale to mm */
    xquantity res = qty3.rescale(xo::qty::nu::millimeter);

    /* 2286mm */
    cerr << "res: " << res << endl;

    /* 12 */
    xquantity qty4 = qty1 / qty2;

    auto res2 = qty4 + 4;

    cerr << "res2: " << res << endl;
}

/** end ex8.cpp */
