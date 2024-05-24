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
            /** @defgroup scaled-unit-type-traits scaled-unit type traits **/
            ///@{

            /** type for representing individual basis-unit scalefactors **/
            using ratio_int_type = typename natural_unit<Int>::ratio_int_type;

            ///@}

        public:
            /** @defgroup scaled-unit-ctors scaled-unit constructors **/
            ///@{

            /** create scaled unit representing a multiple
             *  @p outer_scale_factor * @p sqrt(outer_scale_sq)
             *  of natural unit @p nat_unit
             **/
            constexpr scaled_unit(const natural_unit<Int> & nat_unit,
                                  OuterScale outer_scale_factor,
                                  double outer_scale_sq)
                : natural_unit_{nat_unit},
                  outer_scale_factor_{outer_scale_factor},
                  outer_scale_sq_{outer_scale_sq}
                {}

            ///@}

            /** @defgroup scaled-unit-access-methods scaled-unit access methods **/
            ///@{

            /** always true for scaled_unit **/
            static constexpr bool is_scaled_unit_type_v = true;

            /** always true for scaled_unit **/
            constexpr bool is_scaled_unit_type() const { return true; }

            /** true iff scaled unit can be faithfully represented by a @ref natural_unit **/
            constexpr bool is_natural() const {
                return (outer_scale_factor_ == OuterScale(1) && (outer_scale_sq_ == 1.0));
            }

            /** true if this scaled unit has no dimension **/
            constexpr bool is_dimensionless() const { return natural_unit_.is_dimensionless(); }

            /** get number of distinct native dimensions present.
             *  e.g. for unit Newton = 1 kg.m.s^-2, n_bpu would be 3,
             *  with {mass, distance, time} present.
             *  Note that this value does not count exponents
             **/
            constexpr std::size_t n_bpu() const { return natural_unit_.n_bpu(); }

            ///@}

            /** @defgroup scaled-unit-general-methods scaled-unit access methods **/
            ///@{

            /** return reciprocal of this unit. **/
            constexpr scaled_unit reciprocal() const {
                return scaled_unit(natural_unit_.reciprocal(),
                                   1 / outer_scale_factor_,
                                   1.0 / outer_scale_sq_);
            }

            /** get bpu for dimension @p d.  if d isn't present,  construct bpu with 0 power **/
            constexpr bpu<Int> lookup_dim(dimension d) const {
                return natural_unit_.lookup_dim(d);
            }

            /** return @p i'th bpu associated with this unit **/
            constexpr bpu<Int> & operator[](std::size_t i) { return natural_unit_[i]; }
            /** return @p i'th bpu associated with this unit (const version) **/
            constexpr const bpu<Int> & operator[](std::size_t i) const { return natural_unit_[i]; }

            ///@}

        public: /* public members so scaled_unit instance can be a non-type template parameter (a structural type) */

            /** @defgroup scaled-unit-instance-vars **/
            ///@{

            /** scale factor multiplying @ref natural_unit_ **/
            OuterScale outer_scale_factor_;

            /** squared scale factor multiplying @ref natural_unit_ **/
            double outer_scale_sq_;

            /** natural unit term in this scaled unit **/
            natural_unit<Int> natural_unit_;

            ///@}
        };

        // TODO: comparison operators

        namespace detail {
            /** promote natural unit to scaled unit (with unit outer scalefactors) **/
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
                return detail::su_promote(natural_unit<std::int64_t>::from_bu(bu, power));
            }

            /** @defgroup scaled-unit-dimensionless scaled-unit dimensionless constant **/
            ///@{

            /** dimensionless unit; equivalent to 1 **/
            constexpr auto dimensionless    = detail::su_promote(natural_unit<std::int64_t>());

            ///@}

            // ----- mass units -----

            /** @defgroup scaled-unit-mass scaled-unit mass units **/
            ///@{

            /** unit of 10^-12 grams **/
            constexpr auto picogram         = su_from_bu(detail::bu::picogram);
            /** unit of 10^-9 grams **/
            constexpr auto nanogram         = su_from_bu(detail::bu::nanogram);
            /** unit of 10^-6 grams **/
            constexpr auto microgram        = su_from_bu(detail::bu::microgram);
            /** unit of 10^-3 grams **/
            constexpr auto milligram        = su_from_bu(detail::bu::milligram);
            /** unit of 1 gram **/
            constexpr auto gram             = su_from_bu(detail::bu::gram);
            /** unit of 10^3 grams **/
            constexpr auto kilogram         = su_from_bu(detail::bu::kilogram);
            /** unit of 1 metric tonne = 10^3 kg **/
            constexpr auto tonne            = su_from_bu(detail::bu::tonne);
            /** unit of 10^3 tonnes = 10^6 kg **/
            constexpr auto kilotonne        = su_from_bu(detail::bu::kilotonne);
            /** unit of 10^6 tonnes = 10^9 kg **/
            constexpr auto megatonne        = su_from_bu(detail::bu::megatonne);
            /** unit of 10^9 tonnes = 10^12 kg **/
            constexpr auto gigatonne        = su_from_bu(detail::bu::gigatonne);

            ///@}

            // ----- distance units -----

            /** @defgroup scaled-unit-distance scaled-unit distance units **/
            ///@{

            /** unit of 10^-12 meters **/
            constexpr auto picometer        = su_from_bu(detail::bu::picometer);
            /** unit of 10^-9 meters **/
            constexpr auto nanometer        = su_from_bu(detail::bu::nanometer);
            /** unit of 10^-6 meters **/
            constexpr auto micrometer       = su_from_bu(detail::bu::micrometer);
            /** unit of 10^-3 meters **/
            constexpr auto millimeter       = su_from_bu(detail::bu::millimeter);
            /** unit of 1 meter **/
            constexpr auto meter            = su_from_bu(detail::bu::meter);
            /** unit of 10^3 meters **/
            constexpr auto kilometer        = su_from_bu(detail::bu::kilometer);
            /** unit of 10^6 meters (not commonly used) **/
            constexpr auto megameter        = su_from_bu(detail::bu::megameter);
            /** unit of 10^9 meters (not commonly used) **/
            constexpr auto gigameter        = su_from_bu(detail::bu::gigameter);

            /** unit of 1 light-second = distance light travels in a vacuum in 1 second **/
            constexpr auto lightsecond      = su_from_bu(detail::bu::lightsecond);
            /** unit of 1 astronomical unit, for approximate radius of earth orbit **/
            constexpr auto astronomicalunit = su_from_bu(detail::bu::astronomicalunit);

            /** unit of 1 inch = 1/12 feet **/
            constexpr auto inch             = su_from_bu(detail::bu::inch);
            /** unit of 1 foot = 0.3048 meters **/
            constexpr auto foot             = su_from_bu(detail::bu::foot);
            /** unit of 1 yard = 3 feet **/
            constexpr auto yard             = su_from_bu(detail::bu::yard);
            /** unit of 1 mile = 1760 yards **/
            constexpr auto mile             = su_from_bu(detail::bu::mile);

            ///@}

            // ----- time units -----

            /** @defgroup scaled-unit-time scaled-unit time units **/
            ///@{

            /** unit of 1 picosecond = 10^-12 seconds **/
            constexpr auto picosecond       = su_from_bu(detail::bu::picosecond);
            /** unit of 1 nanosecond = 10^-9 seconds **/
            constexpr auto nanosecond       = su_from_bu(detail::bu::nanosecond);
            /** unit of 1 microseccond = 10^-6 seconds **/
            constexpr auto microsecond      = su_from_bu(detail::bu::microsecond);
            /** unit of 1 millisecond = 10^-3 seconds **/
            constexpr auto millisecond      = su_from_bu(detail::bu::millisecond);
            /** unit of 1 second **/
            constexpr auto second           = su_from_bu(detail::bu::second);
            /** unit of 1 minute **/
            constexpr auto minute           = su_from_bu(detail::bu::minute);
            /** unit of 1 hour **/
            constexpr auto hour             = su_from_bu(detail::bu::hour);
            /** unit for a 24-hour day **/
            constexpr auto day              = su_from_bu(detail::bu::day);
            /** unit for a week comprising exactly 7 24-hour days **/
            constexpr auto week             = su_from_bu(detail::bu::week);
            /** unit for a 30-day month **/
            constexpr auto month            = su_from_bu(detail::bu::month);
            /** unit for a year containing exactly 365.25 24-hour days **/
            constexpr auto year             = su_from_bu(detail::bu::year);
            /** unit for a 'year' containing exactly 250 24-hour days.
             *  (approximates the number of business days in a year)
             **/
            constexpr auto year250          = su_from_bu(detail::bu::year250);
            /** unit for a 'year' containing exactly 360 24-hour days **/
            constexpr auto year360          = su_from_bu(detail::bu::year360);
            /** unit for a 'year' containing exactly 365 24-hour days **/
            constexpr auto year365          = su_from_bu(detail::bu::year365);

            ///@}

            /** @defgroup scaled-unit-misc scaled-unit miscellaneous units **/
            ///@{

            // ----- currency -----

            /** generic currency unit **/
            constexpr auto currency         = su_from_bu(detail::bu::currency);

            // ----- price - ---

            /** generic price unit **/
            constexpr auto price            = su_from_bu(detail::bu::price);

            ///@}

            // ----- volatility units -----

            /** @defgroup scaled-unit-volatility scaled-unit volatility units **/
            ///@{

            /** volatility, in 30-day units **/
            constexpr auto volatility_30d   = su_from_bu(detail::bu::month,
                                                         power_ratio_type(-1,2));
            /** volatility, in 250-day 'annual' units **/
            constexpr auto volatility_250d  = su_from_bu(detail::bu::year250,
                                                         power_ratio_type(-1,2));
            /** volatility, in 360-day 'annual' units **/
            constexpr auto volatility_360d  = su_from_bu(detail::bu::year360,
                                                         power_ratio_type(-1,2));
            /** volatility, in 365-day 'annual' units **/
            constexpr auto volatility_365d  = su_from_bu(detail::bu::year365,
                                                         power_ratio_type(-1,2));
            ///@}
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

        /** @defgroup scaled-unit-operators **/
        ///@{

        /** Multiply scaled_unit instances @p x_unit and @p y_unit.
         *  Result is a scaled_unit for the product dimension.
         *  For each basis dimension, result will prioritize scale from @p x_unit ahead of @p y_unit.
         **/
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

        /** Divide scaled_unit instances @p x_unit by @p y_unit.
         *  Result is a scaled_unit for the quotient dimension.
         *  For each basis dimension, result will prioritize scale from @p x_unit ahead of @p y_unit.
         **/
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

        ///@}
    } /*namespace qty*/
} /*namespace xo*/

/** end scaled_unit.hpp **/
