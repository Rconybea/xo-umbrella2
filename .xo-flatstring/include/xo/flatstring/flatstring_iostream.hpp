/** @file flatstring_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "flatstring.hpp"
#include <ostream>
//#include <cstdint>

namespace xo {
    /** @brief print flatstring on stream os.
     *
     **/
    template <std::size_t N>
    void
    print_flatstring (std::ostream & os, const flatstring<N> & x) {
        os << x.c_str();
    }

    /** @brief print flatstring x on stream os.
     *
     *  Example
     *  @code
     *  cout << flatstring("foo");  // outputs "foo"
     *  @endcode
     **/
    template <std::size_t N>
    inline std::ostream &
    operator<< (std::ostream & os, const flatstring<N> & x) {
        print_flatstring(os, x);
        return os;
    }
} /*namespace xo*/

/** end flatstring_iostream.hpp **/
