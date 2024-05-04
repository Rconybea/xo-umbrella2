/** @file ex6.cpp **/

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"
#include <iostream>

int
main () {
    using xo::qty::quantity;
    namespace q = xo::qty::qty;
    using xo::flatstring;

    /* 20% volatility over 360 days */
    auto q1 = q::volatility_360d(0.2);
    /* 10% volatility over 30 days */
    auto q2 = q::volatility_30d(0.1);

    /* 10% volatility per 30 days
     *  ~ (10% * sqrt(360/30)) volatility over 360 days
     *  ~ (10% * 3.4641)
     *  ~ 0.34641yr360^(-1/2)
     */

    auto sum = q1 + q2;
    auto prod = q1 * q2;

    static_assert(sum.abbrev() == flatstring("yr360^(-1/2)"));
    static_assert(prod.abbrev() == flatstring("yr360^-1"));

    std::cerr << "q1: " << q1 << std::endl;
    std::cerr << "q2: " << q2 << std::endl;
    std::cerr << "q1+q2: " << sum << std::endl;
    std::cerr << "q1*q2: " << prod << std::endl;
}

/** end ex6.cpp */
