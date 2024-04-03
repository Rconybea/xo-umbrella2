/** @file ex1.cpp **/

#include "xo/unit/quantity.hpp"
#include <iostream>

int
main () {
    namespace qty = xo::unit::qty;
    using namespace std;

    auto t = qty::milliseconds(10);
    auto m = qty::kilograms(2.5);
    auto a = m / (t*t);

    cerr << "t: " << t << ", m: " << m << ", m.t^-2: " << a << endl;
}

/** end ex1.cpp **/
