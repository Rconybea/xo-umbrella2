/* @file fixed.hpp */

#pragma once

#include <iostream>
#include <cstdint>

namespace xo {
    /* use:
     *   ostream os = ...;
     *
     *   os << fixed(3.1415926, 3)
     *
     * writes
     *   3.142
     *
     * on os,  restoring stream's formatting+precision state
     */
    class fixed {
    public:
        fixed(double x, std::uint16_t prec) : x_{x}, prec_{prec} {}

        /* print this value */
        double x_;
        /* precision */
        std::uint16_t prec_ = 0;
    }; /*fixed*/

    inline std::ostream &
    operator<<(std::ostream & s, fixed const & fx)
    {
        std::ios::fmtflags orig_flags = s.flags();
        std::streamsize orig_p  = s.precision();

        s.flags(std::ios::fixed);
        s.precision(fx.prec_);
        s << fx.x_;

        s.flags(orig_flags);
        s.precision(orig_p);

        return s;
    } /*operator<<*/
} /*namespace xo*/

/* end fixed.hpp */
