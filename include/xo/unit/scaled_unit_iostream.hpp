/** @file scaled_unit_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "scaled_unit2.hpp"
#include <iostream>

namespace xo {
    namespace qty {
        template <typename Int>
        inline std::ostream &
        operator<<(std::ostream & os, const scaled_unit2<Int> & x) {
            os << "<scaled-unit"
               << xtag("bpuv", x.natural_unit_)
               << xtag("outer_scale_exact", x.outer_scale_exact_)
               << xtag("outer_scale_sq", x.outer_scale_sq_)
               << ">";

            return os;
        };
    } /*namespace qty*/
} /*namespace xo*/

/** end scaled_unit_iostream.hpp **/
