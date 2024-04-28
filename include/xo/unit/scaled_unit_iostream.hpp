/** @file scaled_unit_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "scaled_unit.hpp"
#include <iostream>

namespace xo {
    namespace qty {
        template <typename Int, typename OuterScale>
        inline std::ostream &
        operator<<(std::ostream & os,
                   const scaled_unit<Int, OuterScale> & x)
        {
            os << "<scaled-unit"
               << xtag("outer_scale_factor", x.outer_scale_factor_)
               << xtag("outer_scale_sq", x.outer_scale_sq_)
               << xtag("bpuv", x.natural_unit_)
               << ">";

            return os;
        };
    } /*namespace qty*/
} /*namespace xo*/

/** end scaled_unit_iostream.hpp **/
