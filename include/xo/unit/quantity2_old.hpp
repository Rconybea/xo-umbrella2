/** @file quantity.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "bpu_array.hpp"

namespace xo {
    namespace qty {
        /** @class quantity
         *  @brief represent a scalar quantity with attached units.  enforce dimensional consistency.
         *
         *  Constexpr implementation, can compute units at compile time
         **/
        template <typename Repr = double,
                  typename Int = std::int64_t,
                  natural_unit<Int> NaturalUnit>
        class quantity {
        public:
            using repr_type = Repr;
            using unit_type = natural_unit<int>;

        public:
            constexpr quantity(Repr scale)
                : scale_{scale} {}

            constexpr const repr_type & scale() const { return scale_; }
            constexpr unit_type unit() const { return NaturalUnit; }

            constexpr bool is_dimensionless() const { return s_unit.is_dimensionless(); }

            constexpr quantity unit_qty() { return quantity(1); }

        private:
            /** @brief unit (established at compile time) for this quantity **/
            static NaturalUnit s_unit = NaturalUnit;

            /** @brief quantity represents this multiple of unit amount **/
            Repr scale_ = Repr();
        };
    } /*namespace qty*/
} /*namespace xo*/


/** end quantity.hpp **/
