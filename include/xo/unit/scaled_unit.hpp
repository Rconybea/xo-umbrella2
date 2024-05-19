/** @file scaled_unit.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "width2x.hpp"

namespace xo {
    namespace qty {
        /** @class scaled_unit
         *  @brief Represents the product sqrt(outer_scale_sq) * outer_scale_exact * nat_unit
         **/
        template < typename Int,
                   typename OuterScale = ratio::ratio<Int> >
        struct scaled_unit {
            using ratio_int_type = typename natural_unit<Int>::ratio_int_type;

            constexpr scaled_unit(const natural_unit<Int> & nat_unit,
                                  OuterScale outer_scale_factor,
                                  double outer_scale_sq)
                : natural_unit_{nat_unit},
                  outer_scale_factor_{outer_scale_factor},
                  outer_scale_sq_{outer_scale_sq}
                {}

            constexpr bool is_scaled_unit_type() const { return true; }

            constexpr scaled_unit reciprocal() const {
                return scaled_unit(natural_unit_.reciprocal(),
                                   1 / outer_scale_factor_,
                                   1.0 / outer_scale_sq_);
            }

            /** @brief true iff scaled unit can be faithfully represented by a @ref natural_unit **/
            constexpr bool is_natural() const {
                return (outer_scale_factor_ == OuterScale(1) && (outer_scale_sq_ == 1.0));
            }

            constexpr bool is_dimensionless() const { return natural_unit_.is_dimensionless(); }
            constexpr std::size_t n_bpu() const { return natural_unit_.n_bpu(); }

            constexpr bpu<Int> & operator[](std::size_t i) { return natural_unit_[i]; }
            constexpr const bpu<Int> & operator[](std::size_t i) const { return natural_unit_[i]; }

        public: /* need public members so that a scaled_unit instance can be a non-type template parameter (a structural type) */

            natural_unit<Int> natural_unit_;
            OuterScale outer_scale_factor_;
            double outer_scale_sq_;
        };

        namespace detail {
            template <typename Int>
            constexpr auto su_promote(const natural_unit<Int> & bpuv) {
                return scaled_unit<Int>(bpuv,
                                        ratio::ratio<Int>(1, 1),
                                        1.0);
            }
        }

        namespace u {
            /* values here can be used as template arguments to quantity:
             * e.g.
             *   quantity<u:picogram> qty1;
             *   quantity<u:meter/u:second> velocity;
             */

            constexpr auto
            su_from_bu(const basis_unit & bu,
                       const power_ratio_type & power = power_ratio_type(1))
            {
                return detail::su_promote<std::int64_t>(natural_unit<std::int64_t>::from_bu(bu, power));
            }

            constexpr auto picogram         = su_from_bu(detail::bu::picogram);
            constexpr auto nanogram         = su_from_bu(detail::bu::nanogram);
            constexpr auto microgram        = su_from_bu(detail::bu::microgram);
            constexpr auto milligram        = su_from_bu(detail::bu::milligram);
            constexpr auto gram             = su_from_bu(detail::bu::gram);
            constexpr auto kilogram         = su_from_bu(detail::bu::kilogram);
            constexpr auto tonne            = su_from_bu(detail::bu::tonne);
            constexpr auto kilotonne        = su_from_bu(detail::bu::kilotonne);
            constexpr auto megatonne        = su_from_bu(detail::bu::megatonne);
            constexpr auto gigatonne        = su_from_bu(detail::bu::gigatonne);

            constexpr auto picometer        = su_from_bu(detail::bu::picometer);
            constexpr auto nanometer        = su_from_bu(detail::bu::nanometer);
            constexpr auto micrometer       = su_from_bu(detail::bu::micrometer);
            constexpr auto millimeter       = su_from_bu(detail::bu::millimeter);
            constexpr auto meter            = su_from_bu(detail::bu::meter);
            constexpr auto kilometer        = su_from_bu(detail::bu::kilometer);
            constexpr auto megameter        = su_from_bu(detail::bu::megameter);
            constexpr auto gigameter        = su_from_bu(detail::bu::gigameter);

            constexpr auto lightsecond      = su_from_bu(detail::bu::lightsecond);
            constexpr auto astronomicalunit = su_from_bu(detail::bu::astronomicalunit);

            constexpr auto inch             = su_from_bu(detail::bu::inch);
            constexpr auto foot             = su_from_bu(detail::bu::foot);
            constexpr auto yard             = su_from_bu(detail::bu::yard);
            constexpr auto mile             = su_from_bu(detail::bu::mile);

            constexpr auto picosecond       = su_from_bu(detail::bu::picosecond);
            constexpr auto nanosecond       = su_from_bu(detail::bu::nanosecond);
            constexpr auto microsecond      = su_from_bu(detail::bu::microsecond);
            constexpr auto millisecond      = su_from_bu(detail::bu::millisecond);
            constexpr auto second           = su_from_bu(detail::bu::second);
            constexpr auto minute           = su_from_bu(detail::bu::minute);
            constexpr auto hour             = su_from_bu(detail::bu::hour);
            constexpr auto day              = su_from_bu(detail::bu::day);
            constexpr auto week             = su_from_bu(detail::bu::week);
            constexpr auto month            = su_from_bu(detail::bu::month);
            constexpr auto year             = su_from_bu(detail::bu::year);
            constexpr auto year250          = su_from_bu(detail::bu::year250);
            constexpr auto year360          = su_from_bu(detail::bu::year360);
            constexpr auto year365          = su_from_bu(detail::bu::year365);

            constexpr auto volatility_30d   = su_from_bu(detail::bu::month,
                                                         power_ratio_type(-1,2));
            constexpr auto volatility_250d  = su_from_bu(detail::bu::year250,
                                                         power_ratio_type(-1,2));
            constexpr auto volatility_360d  = su_from_bu(detail::bu::year360,
                                                         power_ratio_type(-1,2));
            constexpr auto volatility_365d  = su_from_bu(detail::bu::year365,
                                                         power_ratio_type(-1,2));
        }

        namespace detail {
            template <typename Int,
                      typename Int2x = width2x<Int>,
                      typename OuterScale = ratio::ratio<Int2x>>
            constexpr
            scaled_unit<Int, OuterScale>
            su_product(const natural_unit<Int> & lhs_bpu_array,
                       const natural_unit<Int> & rhs_bpu_array)
            {
                natural_unit<Int2x> prod = lhs_bpu_array.template to_repr<Int2x>();

                /* accumulate product of scalefactors spun off by rescaling
                 * any basis-units in rhs_bpu_array that conflict with the same dimension
                 * in lh_bpu_array
                 */
                auto sfr = (detail::outer_scalefactor_result<Int2x>
                            (OuterScale(1) /*outer_scale_factor*/,
                             1.0 /*outer_scale_sq*/));

                for (std::size_t i = 0; i < rhs_bpu_array.n_bpu(); ++i) {
                    auto sfr2 = nu_product_inplace<Int2x, OuterScale>(&prod,
                                                                      rhs_bpu_array[i].template to_repr<Int2x>());

                    sfr.outer_scale_factor_ = sfr.outer_scale_factor_ * sfr2.outer_scale_factor_;
                    sfr.outer_scale_sq_ *= sfr2.outer_scale_sq_;
                }

                return scaled_unit<Int, OuterScale>(prod.template to_repr<Int>(),
                                                    sfr.outer_scale_factor_,
                                                    sfr.outer_scale_sq_);
            }

            /* use Int2x to accumulate scalefactor
             */
            template < typename Int,
                       typename Int2x = width2x<Int>,
                       typename OuterScale = ratio::ratio<Int2x> >
            constexpr
            scaled_unit<Int, OuterScale>
            su_ratio(const natural_unit<Int> & nu_lhs,
                     const natural_unit<Int> & nu_rhs)
            {
                natural_unit<Int2x> ratio = nu_lhs.template to_repr<Int2x>();

                /* accumulate product of scalefactors spun off by rescaling
                 * any basis-units in rhs_bpu_array that conflict with the same dimension
                 * in lh_bpu_array
                 */
                auto sfr = (detail::outer_scalefactor_result<Int2x, OuterScale>
                            (OuterScale(1) /*outer_scale_factor*/,
                             1.0 /*outer_scale_sq*/));

                for (std::size_t i = 0; i < nu_rhs.n_bpu(); ++i) {
                    auto sfr2 = nu_ratio_inplace<Int2x, OuterScale>(&ratio,
                                                                    nu_rhs[i].template to_repr<Int2x>());

                    /* note: nu_ratio_inplace() reports multiplicative outer scaling factors,
                     *       so multiply is correct here
                     */
                    sfr.outer_scale_factor_ = (sfr.outer_scale_factor_
                                               * sfr2.outer_scale_factor_);
                    sfr.outer_scale_sq_ *= sfr2.outer_scale_sq_;
                }

                return scaled_unit<Int, OuterScale>(ratio.template to_repr<Int>(),
                                                    sfr.outer_scale_factor_,
                                                    sfr.outer_scale_sq_);
            }
        }

        template <typename Int,
                  typename Int2x = detail::width2x_t<Int>>
        inline constexpr scaled_unit<Int>
        operator* (const scaled_unit<Int> & x_unit,
                   const scaled_unit<Int> & y_unit)
        {
            auto rr = detail::su_product<Int, Int2x>(x_unit.natural_unit_,
                                                     y_unit.natural_unit_);

            return (scaled_unit<Int>
                    (rr.natural_unit_,
                     (ratio::ratio<Int2x>(rr.outer_scale_factor_)
                      * ratio::ratio<Int2x>(x_unit.outer_scale_factor_)
                      * ratio::ratio<Int2x>(y_unit.outer_scale_factor_)),
                     rr.outer_scale_sq_ * x_unit.outer_scale_sq_ * y_unit.outer_scale_sq_));
        }

#ifdef OBSOLETE
        template <typename Int,
                  typename Int2x = detail::width2x_t<Int>>
        inline constexpr scaled_unit<Int>
        operator* (const scaled_unit<Int> & x_unit,
                   const natural_unit<Int> & y_unit)
        {
            auto y_unit2 = detail::make_unit_rescale_result<Int>(y_unit);

            return x_unit * y_unit2;
        }

        template <typename Int,
                  typename Int2x = detail::width2x_t<Int>>
        inline constexpr scaled_unit<Int>
        operator* (const natural_unit<Int> & x_unit,
                   const scaled_unit<Int> & y_unit)
        {
            auto x_unit2 = detail::make_unit_rescale_result<Int>(x_unit);

            return x_unit2 * y_unit;
        }
#endif

        template <typename Int,
                  typename Int2x = detail::width2x_t<Int>>
        inline constexpr scaled_unit<Int>
        operator/ (const scaled_unit<Int> & x_unit,
                   const scaled_unit<Int> & y_unit)
        {
            auto rr = detail::su_ratio<Int, Int2x>(x_unit.natural_unit_,
                                                   y_unit.natural_unit_);

            return (scaled_unit<Int>
                    (rr.natural_unit_,
                     (ratio::ratio<Int2x>(rr.outer_scale_factor_)
                      * ratio::ratio<Int2x>(x_unit.outer_scale_factor_)
                      * ratio::ratio<Int2x>(y_unit.outer_scale_factor_)),
                     rr.outer_scale_sq_ * x_unit.outer_scale_sq_ * y_unit.outer_scale_sq_));
        }

#ifdef OBSOLETE
        template <typename Int,
                  typename Int2x = detail::width2x_t<Int>>
        inline constexpr scaled_unit<Int>
        operator/ (const scaled_unit<Int> & x_unit,
                   const natural_unit<Int> & y_unit)
        {
            auto y_unit2 = detail::make_unit_rescale_result<Int>(y_unit);

            return x_unit / y_unit2;
        }

        template <typename Int,
                  typename Int2x = detail::width2x_t<Int>>
        inline constexpr scaled_unit<Int>
        operator/ (const natural_unit<Int> & x_unit,
                   const scaled_unit<Int> & y_unit)
        {
            auto x_unit2 = detail::make_unit_rescale_result<Int>(x_unit);

            return x_unit2 / y_unit;
        }
#endif
    } /*namespace qty*/
} /*namespace xo*/

/** end scaled_unit.hpp **/
