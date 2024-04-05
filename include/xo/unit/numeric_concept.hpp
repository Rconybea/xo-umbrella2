/* @file numeric_concept.hpp */

#pragma once

#include <concepts>

namespace xo {
    namespace unit {
        /** @concept numeric_concept
         *  @brief Concept for values that participate in arithmetic operations (+,-,*,/) and comparisons
         *
         *  Intended to include at least:
         *  - built-in integral and floating-point types
         *  - boost::rational<U>
         *  - std::complex<U>
         *  - xo::unit::quantity<U,R>
         *
         *  This implies we don't require T to be totally ordered,
         *  and don't require (<,<=,>=,>) operators.
         *
         *  Intend numeric_concept to apply to types suitable for
         *  xo::unit::quantity::repr_type.
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
    } /*namespace unit*/
} /*namespace xo*/

/* end numeric_concept.hpp */
