/** @file int128_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "flatstring.hpp"
#include <ostream>

namespace std {
    /* print a 128-bit integer */
    inline std::ostream &
    operator<< (std::ostream & os, __int128 x) {
        os << xo::flatstring<48>::from_int(x);
        return os;
    }
}

#ifdef NOT_USING
namespace xo {
    /* print a 128-bit integer */
    inline std::ostream &
    operator<< (std::ostream & os, __int128 x) {
        os << xo::flatstring<48>::from_int(x);
        return os;
    }
}
#endif

/** end int128_iostream.hpp **/
