/** @file quantity.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "quantity_ops.hpp"
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
            auto /*natural_unit<Int>*/ NaturalUnit,
            typename Repr = double,
            typename Int2x = detail::width2x_t<typename decltype(NaturalUnit)::ratio_int_type> >
        class quantity {
        public:
            using repr_type = Repr;
            using unit_type = decltype(NaturalUnit);
            using ratio_int_type = decltype(NaturalUnit)::ratio_int_type;
            using ratio_int2x_type = Int2x;

        public:
            constexpr quantity() : scale_{0} {}
            explicit constexpr quantity(Repr scale) : scale_{scale} {}

            static constexpr bool always_constexpr_unit = true;

            constexpr const repr_type & scale() const { return scale_; }
            constexpr const unit_type & unit() const { return s_unit; }

            constexpr bool is_dimensionless() const {
                return s_unit.is_dimensionless();
            }

            // unit_qty
            // zero_qty

            constexpr
            auto reciprocal() const {
                return quantity<s_unit.reciprocal(),
                                repr_type,
                                ratio_int2x_type>(1.0 / scale_);
            }

            template <typename Repr2>
            constexpr
            auto with_repr() const {
                return quantity<s_unit,
                                Repr2,
                                ratio_int2x_type>(scale_);
            }

            /* parallel implementation to Quantity<Repr, Int>::rescale(),
             * except that NaturalUnit2 is a compile-time-only template-argument
             *
             * NOTE: constexpr as long as no fractional units involved.
             */
            template <natural_unit<ratio_int_type> NaturalUnit2>
            constexpr
            auto rescale() const {
                /* conversion factor from .unit -> unit2*/
                auto rr = detail::su_ratio<ratio_int_type,
                                           ratio_int2x_type>(NaturalUnit, NaturalUnit2);

                if (rr.natural_unit_.is_dimensionless()) {
                    repr_type r_scale = (((rr.outer_scale_sq_ == 1.0)
                                          ? 1.0
                                          : ::sqrt(rr.outer_scale_sq_))
                                         * rr.outer_scale_factor_.template convert_to<repr_type>()
                                         * this->scale_);
                    return quantity<NaturalUnit2, Repr, Int2x>(r_scale);
                } else {
                        return quantity<NaturalUnit2, Repr, Int2x>(std::numeric_limits<repr_type>::quiet_NaN());
                }
            }

            template <scaled_unit<ratio_int_type> ScaledUnit2>
            constexpr
            auto rescale_ext() const {
                /* conversion factor from .unit -> unit2*/
                auto rr = detail::su_ratio<ratio_int_type,
                                           ratio_int2x_type>(NaturalUnit, ScaledUnit2.natural_unit_);

                if (rr.natural_unit_.is_dimensionless()) {
                    /* NOTE: test for unit .outer_scale_sq values to get constexpr result with c++23
                     *       and integer dimension powers.
                     */
                    repr_type r_scale = ((((rr.outer_scale_sq_ == 1.0)
                                           && (ScaledUnit2.outer_scale_sq_ == 1.0))
                                          ? 1.0
                                          : ::sqrt(rr.outer_scale_sq_ / ScaledUnit2.outer_scale_sq_))
                                         * rr.outer_scale_factor_.template convert_to<repr_type>()
                                         * this->scale_
                                         / ScaledUnit2.outer_scale_factor_.template convert_to<repr_type>());
                    return quantity<ScaledUnit2.natural_unit_, Repr, Int2x>(r_scale);
                } else {
                    return quantity<ScaledUnit2.natural_unit_, Repr, Int2x>(std::numeric_limits<repr_type>::quiet_NaN());
                }
            }

            template <typename Dimensionless>
            requires std::is_arithmetic_v<Dimensionless>
            constexpr auto scale_by(Dimensionless x) const {
                return quantity(x * this->scale_);
            }

            // divide_by
            // divide_into

            // divide
            // add
            // subtract

            /* parallel implementation to Quantity<Repr, Int> */
            template <typename Quantity2>
            static constexpr
            auto compare(const quantity &x, const Quantity2 & y) {
                quantity y2 = y.template rescale<s_unit>();

                return x.scale() <=> y2.scale();
            }

            // operator-
            // operator+=
            // operator-=
            // operator*=
            // operator/=

            constexpr nu_abbrev_type abbrev() const { return s_unit.abbrev(); }

            quantity & operator=(const quantity & x) {
                this->scale_ = x.scale_;
                return *this;
            }

            template <typename Q2>
            requires(quantity_concept<Q2>
                     && Q2::always_constexpr_unit)
            quantity & operator=(const Q2 & x) {
                auto x2 = x.template rescale<s_unit>();

                this->scale_ = x2.scale();

                return *this;
            }

            template <typename Q2>
            requires(quantity_concept<Q2>
                     && Q2::always_constexpr_unit)
            constexpr operator Q2() const {
                return this->template rescale<Q2::s_unit>().template with_repr<typename Q2::repr_type>();
            }

            constexpr operator Repr() const
                requires (NaturalUnit.is_dimensionless())
                {
                    return scale_;
                }

        public: /* need public members so that a quantity instance can be a non-type template parameter (is a structural type) */
            static constexpr natural_unit<ratio_int_type> s_unit = NaturalUnit;

            Repr scale_ = Repr{};
        };

        template <typename Quantity, typename Int, typename Int2x>
        constexpr auto
        rescale(const Quantity & x, const scaled_unit<Int, Int2x> & su) {
            return x.template rescale<su>();
        }

        namespace detail {
            struct quantity_util {
                /* parallel implementation to xquantity<Repr, Int> multiply,
                 * but return type will have dimension computed at compile-time
                 */
                template <typename Q1, typename Q2>
                requires (quantity_concept<Q1>
                          && quantity_concept<Q2>
                          && Q1::always_constexpr_unit
                          && Q2::always_constexpr_unit)
                static constexpr auto multiply(Q1 x, Q2 y) {
                    using r_repr_type = std::common_type_t<typename Q1::repr_type,
                                                           typename Q2::repr_type>;
                    using r_int_type = std::common_type_t<typename Q1::ratio_int_type,
                                                          typename Q2::ratio_int_type>;
                    using r_int2x_type = std::common_type_t<typename Q1::ratio_int2x_type,
                                                            typename Q2::ratio_int2x_type>;

                    constexpr auto rr = detail::su_product<r_int_type, r_int2x_type>(x.unit(), y.unit());

                    r_repr_type r_scale = (((rr.outer_scale_sq_ == 1.0)
                                            ? 1.0
                                            : ::sqrt(rr.outer_scale_sq_))
                                           * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                           * static_cast<r_repr_type>(x.scale())
                                           * static_cast<r_repr_type>(y.scale()));

                    return quantity<rr.natural_unit_,
                                    r_repr_type,
                                    r_int2x_type
                                    >(r_scale);
                }

                template <typename Q1, typename Q2>
                requires (quantity_concept<Q1>
                          && quantity_concept<Q2>
                          && Q1::always_constexpr_unit
                          && Q2::always_constexpr_unit)
                static constexpr auto divide(Q1 x, Q2 y) {
                    using r_repr_type = std::common_type_t<typename Q1::repr_type,
                                                           typename Q2::repr_type>;
                    using r_int_type = std::common_type_t<typename Q1::ratio_int_type,
                                                          typename Q2::ratio_int_type>;
                    using r_int2x_type = std::common_type_t<typename Q1::ratio_int2x_type,
                                                            typename Q2::ratio_int2x_type>;

                    constexpr auto rr = detail::su_ratio<r_int_type, r_int2x_type>(x.unit(), y.unit());

                    r_repr_type r_scale = (((rr.outer_scale_sq_ == 1.0)
                                            ? 1.0
                                            : ::sqrt(rr.outer_scale_sq_))
                                           * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                           * static_cast<r_repr_type>(x.scale())
                                           / static_cast<r_repr_type>(y.scale()));

                    return quantity<rr.natural_unit_,
                                    r_repr_type,
                                    r_int2x_type
                                    >(r_scale);
                }

                template <typename Q1, typename Q2>
                requires(quantity_concept<Q1>
                         && quantity_concept<Q2>
                         && Q1::always_constexpr_unit
                         && Q2::always_constexpr_unit)
                static constexpr auto add(Q1 x, Q2 y) {
                    using r_repr_type = std::common_type_t<typename Q1::repr_type,
                                                           typename Q2::repr_type>;
                    using r_int_type = std::common_type_t<typename Q1::ratio_int_type,
                                                          typename Q2::ratio_int_type>;
                    using r_int2x_type = std::common_type_t<typename Q1::ratio_int2x_type,
                                                            typename Q2::ratio_int2x_type>;
                    /* conversion to get y in same units as x: multiply by y/x */
                    auto rr = detail::su_ratio<r_int_type, r_int2x_type>(y.unit(), x.unit());

                    if (rr.natural_unit_.is_dimensionless()) {
                        r_repr_type r_scale = (static_cast<r_repr_type>(x.scale())
                                               + (::sqrt(rr.outer_scale_sq_)
                                                  * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                                  * static_cast<r_repr_type>(y.scale())));

                        return quantity<x.s_unit, r_repr_type, r_int2x_type>(r_scale);
                    } else {
                        /* units don't match! */
                        return quantity<x.s_unit, r_repr_type, r_int2x_type>(std::numeric_limits<r_repr_type>::quiet_NaN());
                    }
                }

                template <typename Q1, typename Q2>
                requires(quantity_concept<Q1>
                         && quantity_concept<Q2>
                         && Q1::always_constexpr_unit
                         && Q2::always_constexpr_unit)
                static constexpr auto subtract(Q1 x, Q2 y) {
                    using r_repr_type = std::common_type_t<typename Q1::repr_type,
                                                           typename Q2::repr_type>;
                    using r_int_type = std::common_type_t<typename Q1::ratio_int_type,
                                                          typename Q2::ratio_int_type>;
                    using r_int2x_type = std::common_type_t<typename Q1::ratio_int2x_type,
                                                            typename Q2::ratio_int2x_type>;
                    /* conversion to get y in same units as x: multiply by y/x */
                    auto rr = detail::su_ratio<r_int_type, r_int2x_type>(y.unit(), x.unit());

                    if (rr.natural_unit_.is_dimensionless()) {
                        r_repr_type r_scale = (static_cast<r_repr_type>(x.scale())
                                               - (::sqrt(rr.outer_scale_sq_)
                                                  * rr.outer_scale_factor_.template convert_to<r_repr_type>()
                                                  * static_cast<r_repr_type>(y.scale())));

                        return quantity<x.s_unit, r_repr_type, r_int2x_type>(r_scale);
                    } else {
                        /* units don't match! */
                        return quantity<x.s_unit, r_repr_type, r_int2x_type>(std::numeric_limits<r_repr_type>::quiet_NaN());
                    }
                }
            };
        } /*namespace detail*/

        template <auto Unit, typename Q1>
        requires(quantity_concept<Q1>
                 && Q1::always_constexpr_unit)
        constexpr auto
        with_units(const Q1 & x) {
            return x.template rescale_ext<Unit>();
        }

        template <typename Q1, typename Q2, auto Unit = Q2::s_unit>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        with_units_from(const Q1 & x, const Q2 & y)
        {
            return x.template rescale<Unit>();
        }

        template <typename Repr2, typename Q1>
        requires (quantity_concept<Q1>
                  && Q1::always_constexpr_unit)
        constexpr auto
        with_repr(const Q1 & x)
        {
            return x.template with_repr<Repr2>();
        }

        /** note: won't have constexpr result w/ fractional dimension until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        operator* (const Q1 & x, const Q2 & y)
        {
            return detail::quantity_util::multiply(x, y);
        }

        /** note: won't have constexpr result w/ fractional dimension until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        operator/ (const Q1 & x, const Q2 & y)
        {
            return detail::quantity_util::divide(x, y);
        }

        /** note: won't have constexpr result w/ fractional dimension until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        operator+ (const Q1 & x, const Q2 & y)
        {
            return detail::quantity_util::add(x, y);
        }

        /** note: won't have constexpr result w/ fractional dimension until c++26 (when ::sqrt(), ::pow() are constexpr)
         **/
        template <typename Q1, typename Q2>
        requires (quantity_concept<Q1>
                  && quantity_concept<Q2>
                  && Q1::always_constexpr_unit
                  && Q2::always_constexpr_unit)
        constexpr auto
        operator- (const Q1 & x, const Q2 & y)
        {
            return detail::quantity_util::subtract(x, y);
        }

        namespace qty {
            // ----- mass -----

            template <typename Repr>
            inline constexpr auto picograms(Repr x) { return quantity<nu::picogram, Repr>(x); }
            template <typename Repr>
            inline constexpr auto nanograms(Repr x) { return quantity<nu::nanogram, Repr>(x); }
            template <typename Repr>
            inline constexpr auto micrograms(Repr x) { return quantity<nu::microgram, Repr>(x); }
            template <typename Repr>
            inline constexpr auto milligrams(Repr x) { return quantity<nu::milligram, Repr>(x); }
            template <typename Repr>
            inline constexpr auto grams(Repr x) { return quantity<nu::gram, Repr>(x); }
            template <typename Repr>
            inline constexpr auto kilograms(Repr x) { return quantity<nu::kilogram, Repr>(x); }
            template <typename Repr>
            inline constexpr auto tonnes(Repr x) { return quantity<nu::tonne, Repr>(x); }
            template <typename Repr>
            inline constexpr auto kilotonnes(Repr x) { return quantity<nu::kilotonne, Repr>(x); }
            template <typename Repr>
            inline constexpr auto megatonnes(Repr x) { return quantity<nu::megatonne, Repr>(x); }
            template <typename Repr>
            inline constexpr auto gigatonnes(Repr x) { return quantity<nu::gigatonne, Repr>(x); }

            // ----- distance -----

            template <typename Repr>
            inline constexpr auto picometers(Repr x) { return quantity<nu::picometer, Repr>(x); }
            template <typename Repr>
            inline constexpr auto nanometers(Repr x) { return quantity<nu::nanometer, Repr>(x); }
            template <typename Repr>
            inline constexpr auto micrometers(Repr x) { return quantity<nu::micrometer, Repr>(x); }
            template <typename Repr>
            inline constexpr auto millimeters(Repr x) { return quantity<nu::millimeter, Repr>(x); }
            template <typename Repr>
            inline constexpr auto meters(Repr x) { return quantity<nu::meter, Repr>(x); }
            template <typename Repr>
            inline constexpr auto kilometers(Repr x) { return quantity<nu::kilometer, Repr>(x); }
            template <typename Repr>
            inline constexpr auto megameters(Repr x) { return quantity<nu::megameter, Repr>(x); }
            template <typename Repr>
            inline constexpr auto gigameters(Repr x) { return quantity<nu::gigameter, Repr>(x); }

            template <typename Repr>
            inline constexpr auto lightseconds(Repr x) { return quantity<nu::lightsecond, Repr>(x); }
            template <typename Repr>
            inline constexpr auto astronomicalunits(Repr x) { return quantity<nu::astronomicalunit, Repr>(x); }

            static constexpr auto meter = meters(1);

            // ----- time -----

            template <typename Repr>
            inline constexpr auto picoseconds(Repr x) { return quantity<nu::picosecond, Repr>(x); }
            template <typename Repr>
            inline constexpr auto nanoseconds(Repr x) { return quantity<nu::nanosecond, Repr>(x); }
            template <typename Repr>
            inline constexpr auto microseconds(Repr x) { return quantity<nu::microsecond, Repr>(x); }
            template <typename Repr>
            inline constexpr auto milliseconds(Repr x) { return quantity<nu::millisecond, Repr>(x); }

            template <typename Repr>
            inline constexpr auto seconds(Repr x) { return quantity<nu::second, Repr>(x); }

            template <typename Repr>
            inline constexpr auto minutes(Repr x) { return quantity<nu::minute, Repr>(x); }

            template <typename Repr>
            inline constexpr auto hours(Repr x) { return quantity<nu::hour, Repr>(x); }
            template <typename Repr>
            inline constexpr auto days(Repr x) { return quantity<nu::day, Repr>(x); }
            template <typename Repr>
            inline constexpr auto weeks(Repr x) { return quantity<nu::week, Repr>(x); }
            template <typename Repr>
            inline constexpr auto months(Repr x) { return quantity<nu::month, Repr>(x); }
            template <typename Repr>
            inline constexpr auto years(Repr x) { return quantity<nu::year, Repr>(x); }
            template <typename Repr>
            inline constexpr auto year250s(Repr x) { return quantity<nu::year250, Repr>(x); }
            template <typename Repr>
            inline constexpr auto year360s(Repr x) { return quantity<nu::year360, Repr>(x); }
            template <typename Repr>
            inline constexpr auto year365s(Repr x) { return quantity<nu::year365, Repr>(x); }

            static constexpr auto second = seconds(1);

            // ----- volatility -----

            /* variance expressed has dimension 1/t;
             * volatility ~ sqrt(variance),  has dimension 1/sqrt(t)
             */

            template <typename Repr>
            inline constexpr auto volatility_30d(Repr x) { return quantity<nu::volatility_30d, Repr>(x); }
            template <typename Repr>
            inline constexpr auto volatility_250d(Repr x) { return quantity<nu::volatility_250d, Repr>(x); }
            template <typename Repr>
            inline constexpr auto volatility_360d(Repr x) { return quantity<nu::volatility_360d, Repr>(x); }
        }

        /* reminder: see [quantity_ops.hpp] for operator* etc */
    } /*namespace qty*/
} /*namespace xo*/

/** end quantity.hpp **/
