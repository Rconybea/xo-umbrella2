/** @file numeric_concept.hpp **/

#pragma once

#include <concepts>

namespace xo {
    namespace ratio {
        /** @concept numeric_concept
         *  @brief Concept for values that participate in arithmetic operations (+,-,*,/) and comparisons
         *
         *  Intended to include at least:
         *  - built-in integral and floating-point types
         *  - big_int<N> from ctbignum
         *  - boost::rational<U>
         *  - std::complex<U>
         *  - xo::unit::quantity<U,R>
         *
         *  Accepting complex numbers --> we don't require T to be totally ordered,
         *  and don't require (<,<=,>=,>) operators.
         *
         *  Intend numeric_concept to apply to types T suitable for
         *  xo::ratio::ratio<T>
         **/
        template <typename T, typename U = T>
        concept numeric_concept = requires(T x, U y)
        {
            { -x };
            { x - y };
            { x + y };
            { x * y };
            { x / y };
            { x == y };
            { x != y };
        };
    } /*namespace ratio*/
} /*namespace xo*/

/* end numeric_concept.hpp */
