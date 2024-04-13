/** @file stringliteral_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "stringliteral.hpp"
#include <ostream>
//#include <cstdint>

namespace xo {
    /** @brief print stringliteral on stream os.
     *
     **/
    template <std::size_t N>
    void
    print_stringliteral (std::ostream & os, const stringliteral<N> & x) {
        os << x.c_str();
    }

    /** @brief print stringliteral x on stream os.
     *
     *  Example
     *  @code
     *  cout << stringliteral("foo");  // outputs "foo"
     **/
    template <std::size_t N>
    inline std::ostream &
    operator<< (std::ostream & os, const stringliteral<N> & x) {
        print_stringliteral(os, x);
        return os;
    }
} /*namespace xo*/

/** end stringliteral_iostream.hpp **/
