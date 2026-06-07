/* @file pair.hpp */

#pragma once

#include <iostream>
#include <utility>

namespace std {
    template <typename T, typename U>
    inline std::ostream &
    operator<<(std::ostream & os,
               std::pair<T,U> const & x)
    {
        os << "["
           << x.first
           << " "
           << x.second
           << "]";

        return os;
    } /*operator<<*/
} /*namespace std*/

/* end pair.hpp */
