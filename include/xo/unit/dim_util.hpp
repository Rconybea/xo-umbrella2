/* @file dim_util.hpp */

#pragma once


#include "stringliteral.hpp"

namespace xo {
    namespace unit {
        enum class dim {
            /** weight.  native unit = 1 gram **/
            mass,
            /** distance.  native unit = 1 meter **/
            distance,
            /** time. native unit = 1 second **/
            time,
            /** a currency amount. native unit depends on actual currency.
             *  For USD: one US dollar.
             *
             *  NOTE: unit system isn't suitable for multicurrency work:
             *        (1usd + 1eur) is well-defined,  but (1sec + 1m) is not.
             **/
            currency,
            /** a screen price. dimensionless **/
            price,
        };

        enum class native_unit_id {
            gram,
            meter,
            second,
            currency,
            price
        };

        template <dim Dim>
        struct native_unit_for;

        template <>
        struct native_unit_for<dim::mass> { static constexpr auto value = native_unit_id::gram; };

        template <>
        struct native_unit_for<dim::distance> { static constexpr auto value = native_unit_id::meter; };

        template <>
        struct native_unit_for<dim::time> { static constexpr auto value = native_unit_id::second; };

        template <>
        struct native_unit_for<dim::currency> { static constexpr auto value = native_unit_id::currency; };

        template <>
        struct native_unit_for<dim::price> { static constexpr auto value = native_unit_id::price; };

        template <dim Dim>
        constexpr auto native_unit_for_v = native_unit_for<Dim>::value;
    } /*namespace unit*/
} /*namespace xo*/

/* end dim_util.hpp */
