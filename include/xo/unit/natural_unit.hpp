/** @file natural_unit.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "bpu.hpp"
#include <cmath>
#include <cassert>

namespace xo {
    namespace qty {
        using nu_abbrev_type = flatstring<32>;

        template <typename Int>
        class natural_unit;

        namespace detail {
            template <typename Int, typename... Ts>
            constexpr void
            push_bpu_array(natural_unit<Int> * p_target, Ts... args);

            template <typename Int>
            constexpr void
            push_bpu_array(natural_unit<Int> * p_target) {}

            template <typename Int, typename T0, typename... Ts>
            constexpr void
            push_bpu_array(natural_unit<Int> * p_target, T0 && bpu0, Ts... args) {
                p_target->push_back(bpu0);
                push_bpu_array(p_target, args...);
            }
        }

        namespace detail {
            template <typename Int>
            struct nu_maker {
                template <typename... Ts>
                static constexpr natural_unit<Int>
                make_nu(Ts... args) {
                    natural_unit<Int> bpu_array;
                    detail::push_bpu_array(&bpu_array, args...);
                    return bpu_array;
                }
            };
        }

        /** @class natural_unit
         *  @brief an array representing the cartesian product of distinct basis-power-units
         *
         *  1. Quantities are represented as a multiple of a natural unit
         *  2. Each bpu in the array represents a power of a basis dimension,  e.g. "meter" or "second^2".
         *  3. Each bpu in an array has a different dimension id.
         *     For example dim::time, if present, appears once.
         *  4. Basis dimensions can appear in any order.
         *     Order used for constructing abbreviations: will get @c "kg.m" or @c "m.kg"
         *     depending on the orderin of @c dim::distance and @c dim::mass in @c bpu_v_
         **/
        template <typename Int>
        class natural_unit {
        public:
            using ratio_int_type = Int;

        public:
            constexpr natural_unit() : n_bpu_{0} {}

            static constexpr natural_unit from_bu(basis_unit bu) {
                return detail::nu_maker<Int>::make_nu(make_unit_power<Int>(bu));
            }

            constexpr std::size_t n_bpu() const { return n_bpu_; }
            constexpr bool is_dimensionless() const { return n_bpu_ == 0; }

            constexpr bpu<Int> * bpu_v() const { return bpu_v_; }

            constexpr natural_unit reciprocal() const {
                natural_unit retval;

                for (std::size_t i = 0; i < this->n_bpu(); ++i)
                    retval.push_back((*this)[i].reciprocal());

                return retval;
            }

            constexpr nu_abbrev_type abbrev() const {
                nu_abbrev_type retval;

                for (std::size_t i = 0; i < n_bpu_; ++i) {
                    if (i > 0)
                        retval.append(".");
                    retval.append(bpu_v_[i].abbrev(), 0, -1);
                }

                return retval;
            }

            /** @brief remove bpu at position @p p **/
            constexpr void remove_bpu(size_t p) {
                for (std::size_t i = p; i+1 < n_bpu_; ++i)
                    bpu_v_[i] = bpu_v_[i+1];

                --n_bpu_;
            }

            constexpr void push_back(const bpu<Int> & bpu) {
                if (n_bpu_ < n_dim)
                    bpu_v_[n_bpu_++] = bpu;
            }

            constexpr bpu<Int> & operator[](std::size_t i) { return bpu_v_[i]; }
            constexpr const bpu<Int> & operator[](std::size_t i) const { return bpu_v_[i]; }

            template <typename Int2>
            constexpr natural_unit<Int2> to_repr() const {
                natural_unit<Int2> retval;

                std::size_t i = 0;
                for (; i < n_bpu_; ++i)
                    retval.push_back(bpu_v_[i].template to_repr<Int2>());

                return retval;
            }

        public: /* need public members so that a natural_unit instance can be a non-type template parameter (a structural type) */
            /** @brief the number of occupied slots in @c bpu_v_ **/
            std::size_t n_bpu_;

            /** @brief storage for basis power units **/
            bpu<Int> bpu_v_[n_dim];
        };

        template <typename Int>
        constexpr bool
        operator==(const natural_unit<Int> & x, const natural_unit<Int> & y) {
            if (x.n_bpu() != y.n_bpu())
                return false;

            for (std::size_t i = 0, n = x.n_bpu(); i<n; ++i)
                if (x[i] != y[i])
                    return false;

            return true;
        }

        template <typename Int>
        constexpr bool
        operator!=(const natural_unit<Int> & x, const natural_unit<Int> & y) {
            return !(x == y);
        }

        namespace detail {
            /**
             *  Given bpu ~ (b.u)^p:
             *  - b = bpu.scalefactor
             *  - u = bpu.native_dim
             *  - p = bpu.power
             *
             *  want to rewrite in the form a'.(b'.u)^p
             *
             *  Can compute a' exactly iff p is integral.
             *  In that case:
             *    (b.u)^p = ((b/b').b'.u)^p
             *            = (b/b')^p.(b'.u)^p
             *            = a'.(b'.u)^p   with  a' = (b/b')^p
             *
             *  Can write p = p0 + q,  with p0 = floor(p) integral,  q = frac(p) in [0,1)
             *
             *  Then
             *    (b/b')^p = (b/b')^p0 * (b/b')^q
             *
             *  we'll compute:
             *  - (b/b')^p0 exactly (as a ratio)
             *  - (b/b')^q inexactly (as a double)
             **/

            template <typename Int,
                      typename OuterScale = ratio::ratio<Int> >
            struct outer_scalefactor_result {
                constexpr outer_scalefactor_result(const OuterScale & outer_scale_factor,
                                                   double outer_scale_sq)
                    : outer_scale_factor_{outer_scale_factor},
                      outer_scale_sq_{outer_scale_sq} {}

                /* (b/b')^p0 */
                OuterScale outer_scale_factor_;
                /* (b/b')^q -- until c++26 only allow q=0 or q=1/2 */
                double  outer_scale_sq_;
            };

            template <typename Int,
                      typename OuterScale = ratio::ratio<Int> >
            struct bpu2_rescale_result {
                constexpr bpu2_rescale_result(const bpu<Int> & bpu_rescaled,
                                              const OuterScale & outer_scale_factor,
                                              double outer_scale_sq)
                    : bpu_rescaled_{bpu_rescaled},
                      outer_scale_factor_{outer_scale_factor},
                      outer_scale_sq_{outer_scale_sq}
                    {}

                /* (b'.u)^p */
                bpu<Int> bpu_rescaled_;
                /* (b/b')^p0 */
                OuterScale outer_scale_factor_;
                /* [(b/b')^q]^2 -- until c++26 only allow q=0 or q=1/2 */
                double  outer_scale_sq_;
            };

            template < typename Int,
                       typename OuterScale = ratio::ratio<Int> >
            constexpr
            bpu2_rescale_result<Int, OuterScale>
            bpu2_rescale(const bpu<Int> & orig,
                         const scalefactor_ratio_type & new_scalefactor)
            {
                ratio::ratio<Int> mult = (orig.scalefactor() / new_scalefactor);

                /* inv: p_frac in [0, 1) */
                auto p_frac = orig.power().frac();

                /* asof c++26: replace mult_sq with ::pow(mult, p_frac) */
                double mult_sq = std::numeric_limits<double>::quiet_NaN();

                if (p_frac.den() == 1) {
                    mult_sq = 1.0;
                } else if(p_frac.den() == 2) {
                    mult_sq = mult.template convert_to<double>();
                } else {
                    // remaining possibilities not supported until c++26
                }

                ratio::ratio<Int> mult_p = mult.power(orig.power().floor());

                return bpu2_rescale_result<Int, OuterScale>(bpu<Int>(orig.native_dim(),
                                                                     new_scalefactor,
                                                                     orig.power()),
                                                            mult_p.template convert_to<OuterScale>(),
                                                            mult_sq);
            }

            template < typename Int,
                       typename OuterScale >
            constexpr
            outer_scalefactor_result<Int, OuterScale>
            bpu_product_inplace(bpu<Int> * p_target_bpu,
                                const bpu<Int> & rhs_bpu_orig)
            {
                assert(rhs_bpu_orig.native_dim() == p_target_bpu->native_dim());

                bpu2_rescale_result<Int, OuterScale> rhs_bpu_rr
                    = bpu2_rescale<Int, OuterScale>(rhs_bpu_orig,
                                                    p_target_bpu->scalefactor().template convert_to<OuterScale>());

                *p_target_bpu = bpu<Int>(p_target_bpu->native_dim(),
                                         p_target_bpu->scalefactor(),
                                         p_target_bpu->power() + rhs_bpu_orig.power());

                return outer_scalefactor_result<Int>(rhs_bpu_rr.outer_scale_factor_,
                                                     rhs_bpu_rr.outer_scale_sq_);
            }

            template < typename Int,
                       typename OuterScale >
            constexpr
            outer_scalefactor_result<Int, OuterScale>
            bpu_ratio_inplace(bpu<Int> * p_target_bpu,
                              const bpu<Int> & rhs_bpu_orig)
            {
                assert(rhs_bpu_orig.native_dim() == p_target_bpu->native_dim());

                bpu2_rescale_result<Int, OuterScale> rhs_bpu_rr
                    = bpu2_rescale<Int, OuterScale>(rhs_bpu_orig,
                                                    p_target_bpu->scalefactor());

                *p_target_bpu = bpu<Int>(p_target_bpu->native_dim(),
                                         p_target_bpu->scalefactor(),
                                         p_target_bpu->power() - rhs_bpu_orig.power());

                return outer_scalefactor_result<Int, OuterScale>
                    (OuterScale(1) / rhs_bpu_rr.outer_scale_factor_,
                     1.0 / rhs_bpu_rr.outer_scale_sq_);
            }

            template < typename Int,
                       typename OuterScale >
            constexpr
            outer_scalefactor_result<Int, OuterScale>
            nu_product_inplace(natural_unit<Int> * p_target,
                               const bpu<Int> & bpu)
            {
                std::size_t i = 0;
                for (; i < p_target->n_bpu(); ++i) {
                    auto * p_target_bpu = &((*p_target)[i]);

                    if (p_target_bpu->native_dim() == bpu.native_dim()) {
                        outer_scalefactor_result<Int, OuterScale> retval
                            = bpu_product_inplace<Int, OuterScale>(p_target_bpu, bpu);

                        if (p_target_bpu->power().is_zero()) {
                            /* dimension assoc'd with *p_target_bpu has been cancelled */
                            p_target->remove_bpu(i);
                        }

                        return retval;
                    }
                }

                /* control here: i=p_target->n_bpu()
                 * Dimension represented by bpu does not already appear in *p_target.
                 * Adopt bpu's scalefactor
                 */

                p_target->push_back(bpu);

                return outer_scalefactor_result<Int, OuterScale>
                    (OuterScale(1) /*outer_scale_factor*/,
                     1.0 /*outer_scale_sq*/);
            }

            template < typename Int,
                       typename OuterScale = ratio::ratio<Int> >
            constexpr
            outer_scalefactor_result<Int, OuterScale>
            nu_ratio_inplace(natural_unit<Int> * p_target,
                             const bpu<Int> & bpu)
            {
                std::size_t i = 0;
                for (; i < p_target->n_bpu(); ++i) {
                    auto * p_target_bpu = &((*p_target)[i]);

                    if (p_target_bpu->native_dim() == bpu.native_dim()) {
                        outer_scalefactor_result<Int, OuterScale> retval
                            = bpu_ratio_inplace<Int, OuterScale>(p_target_bpu, bpu);

                        if (p_target_bpu->power().is_zero()) {
                            /* dimension assoc'd with *p_target_bpu has been cancelled */
                            p_target->remove_bpu(i);
                        }

                        return retval;
                    }
                }

                /* here: i=p_target->n_bpu()
                 * Dimension represented by bpu does not already appear in *p_target.
                 * Adopt bpu's scalefactor
                 */

                p_target->push_back(bpu.reciprocal());

                return outer_scalefactor_result<Int, OuterScale>
                    (OuterScale(1) /*outer_scale_factor*/,
                     1.0 /*outer_scale_sq*/);
            }

        } /*namespace detail*/

        namespace nu {
            constexpr auto dimensionless = natural_unit<std::int64_t>();

            constexpr auto picogram = natural_unit<std::int64_t>::from_bu(bu::picogram);
            constexpr auto nanogram = natural_unit<std::int64_t>::from_bu(bu::nanogram);
            constexpr auto microgram = natural_unit<std::int64_t>::from_bu(bu::microgram);
            constexpr auto milligram = natural_unit<std::int64_t>::from_bu(bu::milligram);
            constexpr auto gram = natural_unit<std::int64_t>::from_bu(bu::gram);
            constexpr auto kilogram = natural_unit<std::int64_t>::from_bu(bu::kilogram);
            constexpr auto tonne = natural_unit<std::int64_t>::from_bu(bu::tonne);
            constexpr auto kilotonne = natural_unit<std::int64_t>::from_bu(bu::kilotonne);
            constexpr auto megatonne = natural_unit<std::int64_t>::from_bu(bu::megatonne);

            constexpr auto picometer = natural_unit<std::int64_t>::from_bu(bu::picometer);
            constexpr auto nanometer = natural_unit<std::int64_t>::from_bu(bu::nanometer);
            constexpr auto micrometer = natural_unit<std::int64_t>::from_bu(bu::micrometer);
            constexpr auto millimeter = natural_unit<std::int64_t>::from_bu(bu::millimeter);
            constexpr auto meter = natural_unit<std::int64_t>::from_bu(bu::meter);
            constexpr auto kilometer = natural_unit<std::int64_t>::from_bu(bu::kilometer);
            constexpr auto megameter = natural_unit<std::int64_t>::from_bu(bu::megameter);
            constexpr auto gigameter = natural_unit<std::int64_t>::from_bu(bu::gigameter);
            constexpr auto lightsecond = natural_unit<std::int64_t>::from_bu(bu::lightsecond);
            constexpr auto astronomicalunit = natural_unit<std::int64_t>::from_bu(bu::astronomicalunit);

            constexpr auto picosecond = natural_unit<std::int64_t>::from_bu(bu::picosecond);
            constexpr auto nanosecond = natural_unit<std::int64_t>::from_bu(bu::nanosecond);
            constexpr auto microsecond = natural_unit<std::int64_t>::from_bu(bu::microsecond);
            constexpr auto millisecond = natural_unit<std::int64_t>::from_bu(bu::millisecond);
            constexpr auto second = natural_unit<std::int64_t>::from_bu(bu::second);
            constexpr auto minute = natural_unit<std::int64_t>::from_bu(bu::minute);
            constexpr auto hour = natural_unit<std::int64_t>::from_bu(bu::hour);
            constexpr auto day = natural_unit<std::int64_t>::from_bu(bu::day);
            constexpr auto week = natural_unit<std::int64_t>::from_bu(bu::week);
            constexpr auto month = natural_unit<std::int64_t>::from_bu(bu::month);
            constexpr auto year = natural_unit<std::int64_t>::from_bu(bu::year);
            constexpr auto year250 = natural_unit<std::int64_t>::from_bu(bu::year250);
            constexpr auto year360 = natural_unit<std::int64_t>::from_bu(bu::year360);
            constexpr auto year365 = natural_unit<std::int64_t>::from_bu(bu::year365);

            constexpr auto currency = natural_unit<std::int64_t>::from_bu(bu::currency);

            constexpr auto price = natural_unit<std::int64_t>::from_bu(bu::price);
        } /*namespace nu*/
    } /*namespace qty*/
} /*namespace xo*/

/** end natural_unit.hpp **/
