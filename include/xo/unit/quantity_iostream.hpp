/** @file quantity_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity.hpp"
#include "natural_unit_iostream.hpp"

namespace xo {
    namespace qty {
        template < auto NaturalUnit, typename Repr, typename Int2x >
        inline std::ostream &
        operator<< (std::ostream & os,
                    const quantity<NaturalUnit, Repr, Int2x> & x)
        {
            os << x.scale() << x.abbrev();
            return os;
        }

    } /*namespace qty*/

} /*namespace xo*/

/** end quantity_iostream.hpp **/
