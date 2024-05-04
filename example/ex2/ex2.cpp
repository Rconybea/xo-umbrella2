/** @file ex2.cpp **/

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"
#include <iostream>

int
main () {
    namespace q = xo::qty::qty;
    namespace su = xo::qty::su;
    using xo::qty::with_units_from;
    using xo::qty::quantity;
    using xo::flatstring;
    using namespace std;

    constexpr auto t = q::minutes(2);
    constexpr auto d = q::kilometers(2.5);

    constexpr auto t2 = t*t;
    constexpr auto a = d / (t*t);

    cerr << "t: " << t << ", d: " << d
         << ", t^2: " << t2
         << ", d.t^-2: " << a
         << endl;

    constexpr auto a2 = a.template rescale<su::meter / (su::second * su::second)>();

    static_assert(a2.abbrev() == flatstring("m.s^-2"));

    cerr << "a2: " << a2 << endl;

    constexpr auto au = q::meter / (q::second * q::second);
    constexpr auto a3 = with_units_from(a, au);

    static_assert(a3.abbrev() == flatstring("m.s^-2"));

    cerr << "a3: " << a3 << endl;
}

/** end ex2.cpp **/
