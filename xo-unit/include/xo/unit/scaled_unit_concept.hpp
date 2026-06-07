/** @file scaled_unit_concept.hpp **/

#pragma once

#include <concepts>

namespace xo {
    namespace qty {
        template <typename ScaledUnit>
        concept scaled_unit_concept = requires(ScaledUnit su)
        {
            typename ScaledUnit::ratio_int_type;

            { su.is_scaled_unit_type() } -> std::same_as<bool>;
            { su.is_natural() } -> std::same_as<bool>;
            { su.is_dimensionless() } -> std::same_as<bool>;

        } && ScaledUnit::is_scaled_unit_type_v;
    } /*namespace qty*/
} /*namespace xo*/


/** end scaled_unit_concept.hpp **/
