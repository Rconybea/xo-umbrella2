/** @file ex5.cpp **/

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"
#include <iostream>

int
main () {
    //namespace u = xo::unit::units;
    namespace q = xo::qty::qty;
    using namespace std;

    /* 20% volatility over 250 days (approx number of trading days in one year) */
    auto q1 = q::volatility_250d(0.2);
    /* 10% volatility over 30 days */
    auto q2 = q::volatility_30d(0.1);

    auto sum = q1 + q2;
    auto prod = q1 * q2;

    cerr << "q1: " << q1 << ", q2: " << q2 << ", q1+q2: " << sum << ", q1*q2: " << prod << endl;
}

/** end ex5.cpp */
