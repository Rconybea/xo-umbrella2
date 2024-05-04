/** @file ex4.cpp **/

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"
#include <iostream>

int
main () {
    namespace q = xo::qty::qty;

    auto t1 = q::milliseconds(1);
    auto t2 = q::minutes(1);

    auto r1 = t1 / with_repr<double>(t2);

    static_assert(r1.is_dimensionless());
    static_assert(!t2.is_dimensionless());

    static_assert(std::same_as<decltype(static_cast<double>(r1)), double>);
    //static_assert(std::same_as<decltype(static_cast<double>(t2)), double>);

    /* r1_value: assignment compiles,  since r1 dimensionless */
    double r1_value = r1;

    /* r2_value: assignment won't compile,  'cannot convert' error */
    //double r2_value = t2;

    std::cerr << "t1: " << t1 << ", t2: " << t2 << ", t1/t2: " << r1 << std::endl;
}

/** end ex4.cpp */
