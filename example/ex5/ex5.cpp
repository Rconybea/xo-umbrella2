/** @file ex5.cpp **/

#include "xo/unit/mpl/quantity.hpp"
#include <iostream>

int
main () {
    namespace u = xo::unit::units;
    namespace qty = xo::unit::qty;
    using namespace std;

    /* 20% volatility over 250 days (approx number of trading days in one year) */
    auto q1 = qty::volatility250d(0.2);
    /* 10% volatility over 30 days */
    auto q2 = qty::volatility30d(0.1);

    auto sum = q1 + q2;
    auto prod = q1 * q2;

    cerr << "q1: " << q1 << ", q2: " << q2 << ", q1+q2: " << sum << ", q1*q2: " << prod << endl;
}

/** end ex5.cpp */
