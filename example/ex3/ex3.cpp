/** @file ex3.cpp **/

#include "xo/unit/mpl/quantity.hpp"
#include <iostream>

int
main () {
    namespace u = xo::unit::units;
    namespace qty = xo::unit::qty;
    using namespace std;

    auto t1 = qty::milliseconds(1);
    auto t2 = qty::minutes(1);

    cerr << "t1: " << t1 << ", t2: " << t2 << ", t1*t2: " << t1*t2 << endl;
}

/** end ex3.cpp **/
