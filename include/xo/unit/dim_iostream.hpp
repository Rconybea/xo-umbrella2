/** @file dim_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "dim_util.hpp"
#include <iostream>

namespace xo {
    namespace qty {
        inline std::ostream &
        operator<<(std::ostream & os, dim x) {
            os << dim2str(x);
            return os;
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end dim_iostream.hpp **/
