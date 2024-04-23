/** @file Quantity.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity2_concept.hpp"
#include "scaled_unit.hpp"
#include "natural_unit.hpp"

namespace xo {
    namespace qty {
        /** @class quantity
         *  @brief represent a scalar quantity with attached units.  enforce dimensional consistency.
         *
         *  Constexpr implementation,  but units are explicitly represented:
         *  sizeof(Quantity2) > sizeof(Repr)
         *
         *  Explicit unit representation allows introducing units at runtime,
         *  for example in python bindings
         *
         *  Require:
         *  - Repr supports numeric operations (+, -, *, /)
         *  - Repr supports conversion from double.
         **/
        template <typename Repr = double,
                  typename Int = std::int64_t>
        class Quantity {
        public:
            using repr_type = Repr;
            using unit_type = natural_unit<Int>;
            using ratio_int_type = Int;

        public:
            constexpr Quantity(Repr scale,
                               const natural_unit<Int> & unit)
                : scale_{scale}, unit_{unit} {}

            constexpr const repr_type & scale() const { return scale_; }
            constexpr const unit_type & unit() const { return unit_; }

            constexpr Quantity unit_qty() const { return Quantity(1, unit_); }

            constexpr Quantity reciprocal() const { return Quantity(1.0 / scale_, unit_.reciprocal()); }

            template <typename OtherQuantity>
            static constexpr
            auto multiply(const Quantity & x, const OtherQuantity & y) {
                using r_repr_type = std::common_type_t<typename Quantity::repr_type,
                                                       typename OtherQuantity::repr_type>;
                using r_int_type = std::common_type_t<typename Quantity::ratio_int_type,
                                                      typename OtherQuantity::ratio_int_type>;

                auto rr = detail::nu_product(x.unit(), y.unit());

                r_repr_type r_scale = (::sqrt(rr.outer_scale_sq_)
                                       * rr.outer_scale_exact_.template to<r_repr_type>()
                                       * static_cast<r_repr_type>(x.scale())
                                       * static_cast<r_repr_type>(y.scale()));

                return Quantity<r_repr_type, r_int_type>(r_scale,
                                                          rr.natural_unit_);
            }

            template <typename Quantity2>
            static constexpr
            auto divide(const Quantity & x, const Quantity2 & y) {
                using r_repr_type = std::common_type_t<typename Quantity::repr_type,
                                                       typename Quantity2::repr_type>;
                using r_int_type = std::common_type_t<typename Quantity::ratio_int_type,
                                                      typename Quantity2::ratio_int_type>;

                auto rr = detail::nu_ratio(x.unit(), y.unit());

                /* note: nu_ratio() reports multiplicative outer scaling factors,
                 *       so multiply is correct here
                 */
                r_repr_type r_scale = (::sqrt(rr.outer_scale_sq_)
                                       * rr.outer_scale_exact_.template to<r_repr_type>()
                                       * static_cast<r_repr_type>(x.scale())
                                       / static_cast<r_repr_type>(y.scale()));

                return Quantity<r_repr_type, r_int_type>(r_scale,
                                                         rr.natural_unit_);
            }


        private:
            /** @brief quantity represents this multiple of a unit amount **/
            Repr scale_ = Repr{};
            /** @brief unit for this quantity **/
            natural_unit<Int> unit_;
        }; /*Quantity2*/

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Repr = double,
                  typename Int = std::int64_t>
        inline constexpr Quantity<Repr, Int>
        unit_qty(const scaled_unit<Int> & u) {
            return Quantity<Repr, Int>
                (u.outer_scale_exact_.template to<double>() * ::sqrt(u.outer_scale_sq_),
                 u.natural_unit_);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename OtherQuantity>
        constexpr auto
        operator* (const Quantity & x, const OtherQuantity & y)
        {
            return Quantity::multiply(x, y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename OtherQuantity>
        constexpr auto
        operator/ (const Quantity & x, const OtherQuantity & y)
        {
            return Quantity::divide(x, y);
        }
    } /*namespace qty*/
} /*namespace xo*/


/** end Quantity.hpp **/
