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
         *  - xo::raio<U>
         *  - xo::unit::quantity<U,R>
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
