/** @file Quantity2_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "Quantity2.hpp"
#include <iostream>

namespace xo {
    namespace qty {
        template <typename Repr = double,
                  typename Int = std::int64_t>
        inline std::ostream &
        operator<< (std::ostream & os,
                    const Quantity2<Repr, Int> & x)
        {
            os << "<qty"
               << xtag("scale", x.scale())
               << xtag("unit", x.unit())
               << ">";

            return os;
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end Quantity2_iostream.hpp **/
