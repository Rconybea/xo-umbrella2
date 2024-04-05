/** @file quantity_concept.hpp **/

#pragma once

#include "unit_concept.hpp"
#include "numeric_concept.hpp"

namespace xo {
    namespace unit {
        template <typename Quantity>
        concept quantity_concept = requires(Quantity qty, typename Quantity::repr_type repr)
        {
            typename Quantity::unit_type;
            typename Quantity::repr_type;

            { qty.scale() } -> std::same_as<typename Quantity::repr_type>;
            { Quantity::unit_cstr() } -> std::same_as<char const *>;
            { Quantity::unit_quantity() } -> std::same_as<Quantity>;
            { Quantity::promote(repr) } -> std::same_as<Quantity>;
        };
    } /*namespace unit*/
} /*namespace xo*/
