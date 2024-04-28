/** @file quantity.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "natural_unit.hpp"
#include "scaled_unit.hpp"

namespace xo {
    namespace qty {
        /** @class quantity
         *  @brief represent a scalar quantity with associated units.
         *
         *  Enforce dimensional consistency at compile time.
         *  sizeof(quantity) == sizeof(Repr).
         **/
        template <
            typename Repr = double,
            typename Int = std::int64_t,
            natural_unit<Int> NaturalUnit = natural_unit<Int>(),
            typename Int2x = detail::width2x<Int>
            >
        class quantity {
        public:
            using repr_type = Repr;
            using unit_type = natural_unit<Int>;
            using ratio_int_type = Int;
            using ratio_int2x_type = Int2x;

        public:
            constexpr quantity() : scale_{0} {}
            explicit constexpr quantity(Repr scale) : scale_{scale} {}

            constexpr const repr_type & scale() const { return scale_; }
            constexpr const unit_type & unit() const { return s_unit; }

            constexpr nu_abbrev_type abbrev() const { return s_unit.abbrev(); }

        public: /* need public members so that a quantity instance can be a non-type template parameter (is a structural type) */
            static constexpr natural_unit<Int> s_unit = NaturalUnit;

            Repr scale_ = Repr{};
        };

        namespace qty {
            inline constexpr auto grams(double x) { return quantity<double, std::int64_t, nu::gram>(x); }
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end quantity.hpp **/
