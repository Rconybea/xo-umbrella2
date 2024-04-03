/** @file ex4.cpp **/

#include "xo/unit/quantity.hpp"
#include <iostream>

int
main () {
    namespace u = xo::unit::units;
    namespace qty = xo::unit::qty;
    using namespace std;

    /* 20% volatility over 250 days (approx number of trading days in one year) */
    auto t1 = qty::milliseconds(1);
    /* 10% volatility over 30 days */
    auto t2 = qty::minutes(1);

    auto r1 = t1 / t2.with_repr<double>();
    auto r2 = t2 / t1.with_repr<double>();

    static_assert(same_as<decltype(r1), double>);
    static_assert(same_as<decltype(r2), double>);

    cerr << "t1: " << t1 << ", t2: " << t2 << ", t1/t2: " << r1 << ", t2/t1: " << r2 << endl;
}

/** end ex4.cpp */
