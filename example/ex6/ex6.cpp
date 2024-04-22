/** @file ex6.cpp **/

#include "xo/unit/mpl/quantity.hpp"
#include <iostream>

int
main () {
    namespace u = xo::unit::units;
    namespace qty = xo::unit::qty;
    using namespace std;

    auto t1 = qty::milliseconds(25.0);
    auto t1_usec = t1.with_unit<u::microsecond>();
    auto t1_sec = t1.with_unit<u::second>();

    cerr << "t1: " << t1 << ", t1_usec: " << t1_usec << ", t1_sec: " << t1_sec << endl;
}

/** end ex6.cpp */
