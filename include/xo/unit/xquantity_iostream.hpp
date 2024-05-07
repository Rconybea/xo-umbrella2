/** @file xquantity_iostream.hpp
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
                    const xquantity<Repr, Int> & x)
        {
            os << x.scale() << x.abbrev();

            return os;
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end xquantity_iostream.hpp **/
