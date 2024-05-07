/** @file ex3.cpp **/

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"
#include <iostream>

int
main () {
    namespace q = xo::qty::qty;
    namespace u = xo::qty::u;
    //namespace nu = xo::qty::nu;
    using xo::qty::quantity;
    using xo::flatstring;
    using namespace std;

    constexpr quantity<u::second> t = q::minutes(2);
    constexpr quantity<u::meter> d = q::kilometers(2.5);

    constexpr auto t2 = t*t;
    constexpr auto a = d / (t*t);

    cerr << "t: " << t << ", d: " << d
         << ", d.t^-2: " << a
         << endl;
}

/** end ex3.cpp **/
