/** @file ex_su.cpp **/

#include "xo/unit/scaled_unit.hpp"
#include "xo/unit/scaled_unit_iostream.hpp"
#include <iostream>

using namespace std;

int
main() {
    using namespace xo::qty;

    constexpr auto u_prod = u::meter * u::kilometer;

    static_assert(u_prod[0].bu() == detail::bu::meter);
    static_assert(u_prod[0].power() == power_ratio_type(2));
    static_assert(u_prod.outer_scale_factor_ == xo::ratio::ratio<int64_t>(1000));
    static_assert(u_prod.outer_scale_sq_ == 1.0);   // used if fractional dimension

    constexpr auto u_div = u::meter / u::kilometer;

    static_assert(u_div.n_bpu() == 0);
    static_assert(u_div.outer_scale_factor_ == xo::ratio::ratio<int64_t>(1,1000));
    static_assert(u_prod.outer_scale_sq_ == 1.0);   // used if fractional dimension

    constexpr auto u2_prod = u::meter * u::hour * u::kilometer * u::minute;

    static_assert(u2_prod.n_bpu() == 2);
    static_assert(u2_prod[0].bu() == detail::bu::meter);
    static_assert(u2_prod[1].bu() == detail::bu::hour);
    // *1000 from converting kilometers -> meters
    // /60   from converting minutes -> hours
    // 1000/60 = 50/3 in lowest terms
    static_assert(u2_prod.outer_scale_factor_ == xo::ratio::ratio<int64_t>(50,3));   // used if fractional dimension
    static_assert(u2_prod.outer_scale_sq_ == 1.0);   // used if fractional dimension
}

/** end ex_su.cpp **/
