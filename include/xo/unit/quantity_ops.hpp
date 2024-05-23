/** @file quantity_ops.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity_concept.hpp"

namespace xo {
    namespace qty {
        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires quantity_concept<Quantity> && quantity_concept<Quantity2>
        constexpr auto
        operator== (const Quantity & x, const Quantity2 & y)
        {
            return (Quantity::compare(x, y) == 0);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires quantity_concept<Quantity> && quantity_concept<Quantity2>
        constexpr auto
        operator<=> (const Quantity & x, const Quantity2 & y)
        {
            return Quantity::compare(x, y);
        }

    } /*namespace qty*/

} /*namespace xo*/


/** end quantity_ops.hpp **/
