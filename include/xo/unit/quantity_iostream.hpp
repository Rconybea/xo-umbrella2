/** @file quantity_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity.hpp"
#include "natural_unit_iostream.hpp"

namespace xo {
    namespace qty {
        template <
            typename Repr = double,
            typename Int = std::int64_t,
            natural_unit<Int> NaturalUnit = natural_unit<Int>(),
            typename Int2x = detail::width2x<Int>
            >
        inline std::ostream &
        operator<< (std::ostream & os,
                    const quantity<Repr, Int, NaturalUnit, Int2x> & x)
        {
            os << x.scale() << x.abbrev();
            return os;
        }

    } /*namespace qty*/

} /*namespace xo*/

/** end quantity_iostream.hpp **/
