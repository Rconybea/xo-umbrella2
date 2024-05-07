/** @file quantity_concept.hpp **/

#pragma once

//#include "unit_concept.hpp"
#include "numeric_concept.hpp"

namespace xo {
    namespace qty {
        template <typename Quantity>
        concept quantity_concept = requires(Quantity qty, typename Quantity::repr_type repr)
        {
            typename Quantity::unit_type;
            typename Quantity::repr_type;

            //{ Quantity::multiply(qty, qty) };

            { qty.scale() } -> std::same_as<const typename Quantity::repr_type &>;
            { qty.unit() } -> std::same_as<const typename Quantity::unit_type &>;
            //{ Quantity::unit_cstr() } -> std::same_as<char const *>;
            //{ Quantity::unit_quantity() } -> std::same_as<Quantity>;
            //{ Quantity::promote(repr) } -> std::same_as<Quantity>;
        } && (true //unit_concept<typename Quantity::unit_type>
              && numeric_concept<typename Quantity::repr_type>);
    } /*namespace qty*/
} /*namespace xo*/

/* end quantity_concept.hpp */
