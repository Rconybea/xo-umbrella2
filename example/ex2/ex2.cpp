/** @file ex2.cpp **/

#include "xo/unit/quantity.hpp"
#include <iostream>

int
main () {
    namespace u = xo::obs::units;
    namespace qty = xo::obs::qty;
    using xo::obs::quantity;
    using namespace std;

    quantity<u::second> t = qty::milliseconds(10);
    quantity<u::gram> m = qty::kilograms(2.5);
    auto a = m / (t*t);

    cerr << "t: " << t << ", m: " << m << ", m.t^-2: " << a << endl;
}

/** end ex2.cpp **/
