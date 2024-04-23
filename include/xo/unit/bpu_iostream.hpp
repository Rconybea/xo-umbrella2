/** @file native_bpu2_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/ratio/ratio_iostream.hpp"
#include "bpu.hpp"
#include <iostream>

namespace xo {
    namespace qty {
        template <typename Int>
        inline std::ostream &
        operator<<(std::ostream & os, const bpu<Int> & x) {
            os << "<bpu"
               << xtag("dim", x.native_dim())
               << xtag("mult", x.scalefactor())
               << xtag("pwr", x.power())
               << ">";

            return os;
        }
    } /*namespace qty*/
} /*namespace xo*/


/** end native_bpu2_iostream.hpp **/
