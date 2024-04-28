/** @file scaled_unit2.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "natural_unit.hpp"

namespace xo {
    namespace qty {
        /** @class scaled_unit
         *  @brief Represents the product sqrt(outer_scale_sq) * outer_scale_exact * nat_unit
         **/
        template < typename Int,
                   typename OuterScale = ratio::ratio<Int> >
        struct scaled_unit {
            constexpr scaled_unit(const natural_unit<Int> & nat_unit,
                                  OuterScale outer_scale_factor,
                                  double outer_scale_sq)
                : natural_unit_{nat_unit},
                  outer_scale_factor_{outer_scale_factor},
                  outer_scale_sq_{outer_scale_sq}
                {}

            constexpr scaled_unit reciprocal() const {
                return scaled_unit(natural_unit_.reciprocal(),
                                   1 / outer_scale_factor_,
                                   1.0 / outer_scale_sq_);
            }

            natural_unit<Int> natural_unit_;
            OuterScale outer_scale_factor_;
            double outer_scale_sq_;
        };

        namespace detail {
            template <typename Int>
            constexpr auto make_unit_rescale_result(const natural_unit<Int> & bpuv) {
                return scaled_unit<Int>(bpuv,
                                        ratio::ratio<Int>(1, 1),
                                        1.0);
            }
        }

        namespace su2 {
            constexpr auto nanogram = detail::make_unit_rescale_result<std::int64_t>(nu::nanogram);
            constexpr auto microgram = detail::make_unit_rescale_result<std::int64_t>(nu::microgram);
        }

        namespace detail {
            template <typename Int>
            struct width2x;

            template <>
            struct width2x<std::int16_t> {
                using type = std::int32_t;
            };

            template <>
            struct width2x<std::int32_t> {
                using type = std::int64_t;
            };

            template <>
            struct width2x<std::int64_t> {
                using type = __int128_t;
            };

            template <typename Int>
            using width2x_t = width2x<Int>::type;

#ifdef NOT_USING
            template <typename Int>
            constexpr
            detail::bpu2_rescale_result<Int>
            bpu2_product(const bpu<Int> & lhs_bpu,
                         const bpu<Int> & rhs_bpu)
            {
                assert(lhs_bpu.native_dim() == rhs_bpu.native_dim());

                bpu<Int> prod_bpu = lhs_bpu;
                auto rr = bpu_product_inplace(&prod_bpu, rhs_bpu);

                return bpu2_rescale_result<Int>(prod_bpu,
                                                rr.outer_scale_exact_,
                                                rr.outer_scale_sq_);
            }
#endif

#ifdef NOT_USING
            template <typename Int>
            constexpr
            scaled_unit<Int>
            su_bpu_product(const natural_unit<Int> & lhs_bpu_array,
                           const bpu<Int> & rhs_bpu)
            {
                natural_unit<Int> prod = lhs_bpu_array;
                auto rr = nu_product_inplace(&prod, rhs_bpu);

                return scaled_unit<Int>(prod,
                                        rr.outer_scale_exact_,
                                        rr.outer_scale_sq_);
            };
#endif

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
    } /*namespace qty*/
} /*namespace xo*/

/** end scaled_unit2.hpp **/
