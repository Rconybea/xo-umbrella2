/** @file promoter.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "mpl/unit.hpp"

namespace xo {
    namespace unit {
        /** @class promoter
         *
         *  Auxiliary class driver for quantity::promote().
         *  promoter has two specializations:
         *  1. if Unit is dimensionless, @c promoter::promote() is the identity function.
         *     This has the effect of collapsing dimensionless quantities to their representation.
         *  2. if Unit has at least one non-empty dimension,
         *     @c promoter::promote() builds an xo::unit::quantity instance
         **/
        template <typename Unit, typename Repr, bool Dimensionless = dimensionless_v<Unit> >
        struct promoter;

        template <typename Unit, typename Repr>
        class quantity;

        /* collapse dimensionless quantity to its repr_type> */
        template <typename Unit, typename Repr>
        struct promoter<Unit, Repr, /*Dimensionless*/ true> {
            static constexpr Repr promote(Repr x) { return x; };
        };

        template <typename Unit, typename Repr>
        struct promoter<Unit, Repr, /*Dimensionless*/ false> {
            static constexpr quantity<Unit, Repr> promote(Repr x) { return quantity<Unit, Repr>(x); }
        };
    } /*namespace unit*/
} /*namespace xo*/

/** end promoter.hpp **/
