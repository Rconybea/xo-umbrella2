/** @file xquantity.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity_ops.hpp"
#include "scaled_unit.hpp"
#include "natural_unit.hpp"

namespace xo {
    namespace qty {
        /** @class xquantity
         *  @brief represent a scalar quantity with polymorphic units.
         *
         *  - @p Repr type used represent a dimensionless multiple of a natural unit.
         *
         *  Constexpr implementation,  but units are explicitly represented:
         *  @code
         *  sizeof(Quantity2) > sizeof(Repr)
         *  @endcode
         *
         *  Explicit unit representation allows introducing units at runtime,
         *  for example in python bindings.
         *  See for example <a href="https://github.com/rconybea/xo-pyutil">xo-pyutil</a>
         *
         *  See @ref quantity for implementation with units established at compile time
         *
         *  Require:
         *  - Repr supports numeric operations (+, -, *, /)
         *  - Repr supports conversion from double.
         **/
        template <typename Repr = double,
                  typename Int = std::int64_t,
                  typename Int2x = detail::width2x_t<Int>>
        class xquantity {
        public:
            using repr_type = Repr;
            using unit_type = natural_unit<Int>;
            using ratio_int_type = Int;
            using ratio_int2x_type = Int2x;

        public:
            /* zero, dimensionless */
            constexpr xquantity()
                : scale_{0}, unit_{natural_unit<Int>()} {}
            constexpr xquantity(Repr scale,
                                const natural_unit<Int> & unit)
                : scale_{scale}, unit_{unit} {}

            static constexpr bool always_constexpr_unit = false;

            constexpr const repr_type & scale() const { return scale_; }
            constexpr const unit_type & unit() const { return unit_; }

            constexpr bool is_dimensionless() const { return unit_.is_dimensionless(); }

            constexpr xquantity unit_qty() const { return xquantity(1, unit_); }
            constexpr xquantity zero_qty() const { return xquantity(0, unit_); }
            constexpr xquantity reciprocal() const { return xquantity(1.0 / scale_, unit_.reciprocal()); }

            constexpr
            auto rescale(const natural_unit<Int> & unit2) const {
                /* conversion factor from .unit -> unit2*/
                auto rr = detail::su_ratio<ratio_int_type,
                                           ratio_int2x_type>(this->unit_, unit2);

                if (rr.natural_unit_.is_dimensionless()) {
                    repr_type r_scale = (::sqrt(rr.outer_scale_sq_)
                                         * rr.outer_scale_factor_.template convert_to<repr_type>()
                                         * this->scale_);
                    return xquantity(r_scale, unit2);
                } else {
                    return xquantity(std::numeric_limits<repr_type>::quiet_NaN(), unit2);
                }
            }

            template <typename Dimensionless>
            requires std::is_arithmetic_v<Dimensionless>
            constexpr auto scale_by(Dimensionless x) const {
                return xquantity(x * this->scale_, this->unit_);
            }

            template <typename Dimensionless>
            requires std::is_arithmetic_v<Dimensionless>
            constexpr auto divide_by(Dimensionless x) const {
                return xquantity(this->scale_ / x, this->unit_);
            }

            template <typename Dimensionless>
            requires std::is_arithmetic_v<Dimensionless>
            constexpr auto divide_into(Dimensionless x) const {
                return xquantity(x / this->scale_, this->unit_.reciprocal());
            }

            template <typename Quantity2>
            static constexpr
            auto multiply(const xquantity & x, const Quantity2 & y) {
                using r_repr_type = std::common_type_t<typename xquantity::repr_type,
                                                       typename Quantity2::repr_type>;
                using r_int_type = std::common_type_t<typename xquantity::ratio_int_type,
                                                      typename Quantity2::ratio_int_type>;
                using r_int2x_type = std::common_type_t<typename xquantity::ratio_int2x_type,
                                                        typename Quantity2::ratio_int2x_type>;

                auto rr = detail::su_product<r_int_type, r_int2x_type>(x.unit(), y.unit());

                r_repr_type r_scale = (::sqrt(rr.outer_scale_sq_)
                                       * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                       * static_cast<r_repr_type>(x.scale())
                                       * static_cast<r_repr_type>(y.scale()));

                return xquantity<r_repr_type, r_int_type>(r_scale,
                                                          rr.natural_unit_);
            }

            template <typename Quantity2>
            static constexpr
            auto divide(const xquantity & x, const Quantity2 & y) {
                using r_repr_type = std::common_type_t<typename xquantity::repr_type,
                                                       typename Quantity2::repr_type>;
                using r_int_type = std::common_type_t<typename xquantity::ratio_int_type,
                                                      typename Quantity2::ratio_int_type>;
                using r_int2x_type = std::common_type_t<typename xquantity::ratio_int2x_type,
                                                        typename Quantity2::ratio_int2x_type>;

                auto rr = detail::su_ratio<r_int_type, r_int2x_type>(x.unit(), y.unit());

                /* note: su_ratio() reports multiplicative outer scaling factors,
                 *       so multiply is correct here
                 */
                r_repr_type r_scale = (::sqrt(rr.outer_scale_sq_)
                                       * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                       * static_cast<r_repr_type>(x.scale())
                                       / static_cast<r_repr_type>(y.scale()));

                return xquantity<r_repr_type, r_int_type>(r_scale,
                                                         rr.natural_unit_);
            }

            template <typename Quantity2>
            static constexpr
            auto add(const xquantity & x, const Quantity2 & y) {
                using r_repr_type = std::common_type_t<typename xquantity::repr_type,
                                                       typename Quantity2::repr_type>;
                using r_int_type = std::common_type_t<typename xquantity::ratio_int_type,
                                                      typename Quantity2::ratio_int_type>;
                using r_int2x_type = std::common_type_t<typename xquantity::ratio_int2x_type,
                                                        typename Quantity2::ratio_int2x_type>;

                /* conversion to get y in same units as x:  multiply by y/x */
                auto rr = detail::su_ratio<r_int_type, r_int2x_type>(y.unit(), x.unit());

                if (rr.natural_unit_.is_dimensionless()) {
                    r_repr_type r_scale = (static_cast<r_repr_type>(x.scale())
                                           + (::sqrt(rr.outer_scale_sq_)
                                              * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                              * static_cast<r_repr_type>(y.scale())));

                    return xquantity<r_repr_type, r_int_type>(r_scale, x.unit_.template to_repr<r_int_type>());
                } else {
                    /* units don't match! */
                    return xquantity<r_repr_type, r_int_type>(std::numeric_limits<r_repr_type>::quiet_NaN(),
                                                             x.unit_.template to_repr<r_int_type>());
                }
            }

            template <typename Quantity2>
            static constexpr
            auto subtract(const xquantity & x, const Quantity2 & y) {
                using r_repr_type = std::common_type_t<typename xquantity::repr_type,
                                                       typename Quantity2::repr_type>;
                using r_int_type = std::common_type_t<typename xquantity::ratio_int_type,
                                                      typename Quantity2::ratio_int_type>;
                using r_int2x_type = std::common_type_t<typename xquantity::ratio_int2x_type,
                                                        typename Quantity2::ratio_int2x_type>;

                /* conversion to get y in same units as x:  multiply by y/x */
                auto rr = detail::su_ratio<r_int_type, r_int2x_type>(y.unit(), x.unit());

                if (rr.natural_unit_.is_dimensionless()) {
                    r_repr_type r_scale = (static_cast<r_repr_type>(x.scale())
                                           - (::sqrt(rr.outer_scale_sq_)
                                              * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                              * static_cast<r_repr_type>(y.scale())));

                    return xquantity<r_repr_type, r_int_type>(r_scale, x.unit_.template to_repr<r_int_type>());
                } else {
                    /* units don't match! */
                    return xquantity<r_repr_type, r_int_type>(std::numeric_limits<r_repr_type>::quiet_NaN(),
                                                             x.unit_.template to_repr<r_int_type>());
                }
            }

            template <typename Quantity2>
            static constexpr
            auto compare(const xquantity & x, const Quantity2 & y) {
                xquantity y2 = y.rescale(x.unit_);

                return x.scale() <=> y2.scale();
            }

            xquantity operator-() const {
                return xquantity(-scale_, unit_);
            }

            /* also works with Quantity2 = double, int, .. */
            template <typename Quantity2>
            xquantity & operator*= (const Quantity2 & x) {
                *this = *this * x;
                return *this;
            }

            /* also works with Quantity2 = double, int, .. */
            template <typename Quantity2>
            xquantity & operator/= (const Quantity2 & x) {
                *this = *this / x;
                return *this;
            }

            // TODO: operator+=, operator-=

            constexpr nu_abbrev_type abbrev() const { return unit_.abbrev(); }

        private:
            /** @brief quantity represents this multiple of a unit amount **/
            Repr scale_ = Repr{};
            /** @brief unit for this quantity **/
            natural_unit<Int> unit_;
        }; /*xquantity*/

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Repr = double,
                  typename Int = std::int64_t>
        inline constexpr xquantity<Repr, Int>
        unit_qty(const scaled_unit<Int> & u)
        {
            return xquantity<Repr, Int>
                (u.outer_scale_factor_.template convert_to<double>() * ::sqrt(u.outer_scale_sq_),
                 u.natural_unit_);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Repr = double,
                  typename Int = std::int64_t>
        inline constexpr xquantity<Repr, Int>
        natural_unit_qty(const natural_unit<Int> & nu) {
            return xquantity<Repr, Int>(1.0, nu);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && (!Q1::always_constexpr_unit || !Q2::always_constexpr_unit))
        constexpr auto
        operator* (const Q1 & x, const Q2 & y)
        {
            return Q1::multiply(x, y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires quantity_concept<Quantity> && quantity_concept<Quantity2>
        constexpr auto
        operator/ (const Quantity & x, const Quantity2 & y)
        {
            return Quantity::divide(x, y);
        }

        /** note: doesn not require unit scaling, so constexpr with c++23 **/
        template <typename Quantity, typename Dimensionless>
        requires quantity_concept<Quantity> && std::is_arithmetic_v<Dimensionless>
        constexpr auto
        operator/ (const Quantity & x, Dimensionless y)
        {
            return x.divide_by(y);
        }

        /** note: doesn not require unit scaling, so constexpr with c++23 **/
        template <typename Dimensionless, typename Quantity>
        requires std::is_arithmetic_v<Dimensionless> && quantity_concept<Quantity>
        constexpr auto
        operator/ (Dimensionless x, const Quantity & y)
        {
            return y.divide_into(x);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires quantity_concept<Quantity> && quantity_concept<Quantity2>
        constexpr auto
        operator+ (const Quantity & x, const Quantity2 & y)
        {
            return Quantity::add(x, y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator+ (const Quantity & x, double y)
        {
            return x + Quantity(y, nu::dimensionless);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator+ (double x, const Quantity & y)
        {
            return Quantity(x, nu::dimensionless) + y;
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, typename Quantity2>
        requires quantity_concept<Quantity> && quantity_concept<Quantity2>
        constexpr auto
        operator- (const Quantity & x, const Quantity2 & y)
        {
            return Quantity::subtract(x, y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator- (const Quantity & x, double y)
        {
            return x - Quantity(y, nu::dimensionless);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator- (double x, const Quantity & y)
        {
            return Quantity(x, nu::dimensionless) - y;
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator== (const Quantity & x, double y)
        {
            return (x == Quantity(y, nu::dimensionless));
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity>
        requires quantity_concept<Quantity>
        constexpr auto
        operator== (double x, const Quantity & y)
        {
            return (Quantity(x, nu::dimensionless) == y);
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, double>
        requires quantity_concept<Quantity>
        constexpr auto
        operator<=> (const Quantity & x, double y)
        {
            return Quantity::compare(x, Quantity(y, nu::dimensionless));
        }

        /** note: won't have constexpr result until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Quantity, double>
        requires quantity_concept<Quantity>
        constexpr auto
        operator<=> (double x, const Quantity & y)
        {
            return Quantity::compare(Quantity(x, nu::dimensionless), y);
        }

        namespace unit {
            constexpr auto nanogram = natural_unit_qty(nu::nanogram);
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end xquantity.hpp **/
