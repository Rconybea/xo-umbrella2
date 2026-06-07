/** @file unit2.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "natural_unit.hpp"

namespace xo {
    namespace qty {
        /** @class unit2
         *  @brief represent an arbitrary unit along with dimension details
         *
         *  For example,
         *    kg.m.s^-2 or
         *    (kilogram * meter) / (second * second)
         **/
        template <typename Int>
        using unit2 = natural_unit<Int>;
    } /*namespace qty*/
} /*namespace xo*/

/** end unit2.hpp **/
