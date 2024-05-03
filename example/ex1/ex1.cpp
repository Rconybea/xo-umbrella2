/** @file ex1.cpp **/

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"
#include <iostream>

int
main () {
    namespace q = xo::qty::qty;
    using xo::qty::quantity;
    using namespace std;

    constexpr auto t = q::milliseconds(10);
    constexpr auto m = q::kilograms(2.5);

    constexpr auto t2 = t*t;
#ifdef NOT_YET
    constexpr auto a = m / (t*t);

    static_assert(same_as<decltype(t), quantity<u::millisecond, int>>);
#endif

    cerr << "t: " << t << ", m: " << m
         << ", t^2: " << t2
        //<< ", m.t^-2: " << a
         << endl;
}

/** end ex1.cpp **/
