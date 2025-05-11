/** @file bpu_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "bpu.hpp"
#include "dim_iostream.hpp"
#include "xo/ratio/ratio_iostream.hpp"
#include "xo/indentlog/print/tag.hpp"
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

/** end bpu_iostream.hpp **/
