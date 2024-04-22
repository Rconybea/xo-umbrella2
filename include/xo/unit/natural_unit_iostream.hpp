/** @file natural_unit_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "natural_unit.hpp"
#include <iostream>

namespace xo {
    namespace unit {
        template <typename Int>
        inline std::ostream &
        operator<<(std::ostream & os, const natural_unit<Int> & x) {
            os << "<natural-unit [";
            for (std::size_t i=0; i<x.n_bpu(); ++i) {
                if (i > 0)
                    os << ", ";
                os << x[i];
            }
            os << "]>";
            return os;
        }
    } /*namespace unit*/
} /*namespace xo*/

/** end natural_unit_iostream.hpp **/
