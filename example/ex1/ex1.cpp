/** @file ex1.cpp **/

#include "xo/unit/quantity.hpp"
#include <iostream>

int
main () {
    namespace u = xo::unit::units;
    namespace qty = xo::unit::qty;
    using xo::unit::quantity;
    using namespace std;

    auto t = qty::milliseconds(10);
    auto m = qty::kilograms(2.5);
    auto a = m / (t*t);

    static_assert(same_as<decltype(t), quantity<u::millisecond, int>>);

    cerr << "t: " << t << ", m: " << m << ", m.t^-2: " << a << endl;
}

/** end ex1.cpp **/
