/** @file Quantity2_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xquantity.hpp"
#include "natural_unit_iostream.hpp"
//#include <iostream>

namespace xo {
    namespace qty {
        template <typename Repr = double,
                  typename Int = std::int64_t>
        inline std::ostream &
        operator<< (std::ostream & os,
                    const Quantity<Repr, Int> & x)
        {
            os << x.scale() << x.abbrev();

#ifdef NOT_USING
            os << "<qty"
               << xtag("scale", x.scale())
               << xtag("unit", x.unit())
               << ">";
#endif

            return os;
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end Quantity2_iostream.hpp **/
