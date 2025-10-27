/** @file quantity_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity.hpp"
#include "natural_unit_iostream.hpp"

namespace xo {
    namespace qty {
        template < auto NaturalUnit, typename Repr >
        inline std::ostream &
        operator<< (std::ostream & os,
                    const quantity<NaturalUnit, Repr> & x)
        {
            os << x.scale() << x.abbrev();
            return os;
        }

    } /*namespace qty*/

    namespace print {
#ifndef ppdetail_atomic
        template <auto NaturalUnit, typename Repr>
        struct ppdetail<xo::qty::quantity<NaturalUnit, Repr>> {
            using target_type = xo::qty::quantity<NaturalUnit, Repr>;

            static bool print_pretty(const ppindentinfo & ppii,
                                     const target_type & x) {
                return ppdetail_atomic<target_type>::print_pretty(ppii, x);
            }
        };
#endif
    } /*namespace print*/
} /*namespace xo*/

/** end quantity_iostream.hpp **/
