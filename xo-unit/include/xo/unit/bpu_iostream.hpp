/** @file bpu_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "bpu.hpp"
#include "dim_iostream.hpp"
#include <xo/ratio/ratio_iostream.hpp>
#include <xo/ppsink/tag_ostream.hpp>
#include <iostream>

namespace xo {
    namespace qty {
        template <typename Int>
        inline std::ostream &
        operator<<(std::ostream & os, const bpu<Int> & x) {
            /* function-local: at namespace scope this would be ambiguous with
             * legacy xo::xtag in any TU that also sees xo-indentlog.
             */
            using xo::pp::xtag;

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
