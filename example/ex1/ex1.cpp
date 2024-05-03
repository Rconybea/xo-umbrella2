/** @file ex1.cpp **/

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"
#include <iostream>

int
main () {
    namespace q = xo::qty::qty;
    namespace su = xo::qty::su;
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

    static_assert(std::same_as<decltype(t)::repr_type, double>);
    static_assert(sizeof(t) == sizeof(double));
    static_assert(t.scale() == 2);
    static_assert(t.abbrev() == flatstring("min"));

    static_assert(std::same_as<decltype(d)::repr_type, double>);
    static_assert(sizeof(d) == sizeof(double));
    static_assert(d.scale() == 2.5);
    static_assert(d.abbrev() == flatstring("km"));

    static_assert(std::same_as<decltype(t2)::repr_type, double>);
    static_assert(sizeof(t2) == sizeof(double));
    static_assert(t2.scale() == 4);
    static_assert(t2.abbrev() == flatstring("min^2"));

    static_assert(std::same_as<decltype(a)::repr_type, double>);
    static_assert(sizeof(a) == sizeof(double));
    static_assert(a.scale() == 0.625);
    static_assert(a.abbrev() == flatstring("km.min^-2"));

    constexpr auto a2 = a.rescale<(su::meter / (su::second * su::second))>();

    cerr << "d.t^-2: " << a2 << endl;

    static_assert(a2.abbrev() == flatstring("m.s^-2"));
}

/** end ex1.cpp **/
