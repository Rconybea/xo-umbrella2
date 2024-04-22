/** @file natural_unit.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "bpu2.hpp"
#include <cmath>
#include <cassert>

namespace xo {
    namespace qty {
        /** @class natural_unit
         *  @brief an array representing the cartesian product of distinct basis-power-units
         *
         *  1. Each bpu in the array represents a power of a basis dimension,  e.g. "meter" or "second^2".
         *  2. Each bpu in an array has a different dimension id.
         *     For example dim::time, if present, appears once.
         *  3. Basis dimensions can appear in any order.
         *     Order used for constructing abbreviations: will get @c "kg.m" or @c "m.kg"
         *     depending on the orderin of @c dim::distance and @c dim::mass in @c bpu_v_
         **/
        template <typename Int>
        class natural_unit {
        public:
            using ratio_int_type = Int;

        public:
            constexpr natural_unit() : n_bpu_{0} {}

            constexpr std::size_t n_bpu() const { return n_bpu_; }
            constexpr bpu2<Int> * bpu_v() const { return bpu_v_; }

            constexpr void push_back(const bpu2<Int> & bpu) {
                if (n_bpu_ < n_dim)
                    bpu_v_[n_bpu_++] = bpu;
            }

            constexpr bpu2<Int> & operator[](std::size_t i) { return bpu_v_[i]; }
            constexpr const bpu2<Int> & operator[](std::size_t i) const { return bpu_v_[i]; }

        private:
            /** @brief the number of occupied slots in @c bpu_v_ **/
            std::size_t n_bpu_;

            /** @brief storage for basis power units **/
            bpu2<Int> bpu_v_[n_dim];
        };

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

        template <typename Int>
        struct bpu_array_maker {
            template <typename... Ts>
            static constexpr natural_unit<Int>
            make_bpu_array(Ts... args) {
                natural_unit<Int> bpu_array;
                detail::push_bpu_array(&bpu_array, args...);
                return bpu_array;
            }
        };

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

            template <typename Int>
            struct outer_scalefactor_result {
                constexpr outer_scalefactor_result(const ratio::ratio<Int> & outer_scale_exact,
                                                   double outer_scale_sq)
                    : outer_scale_exact_{outer_scale_exact},
                      outer_scale_sq_{outer_scale_sq} {}

                /* (b/b')^p0 */
                ratio::ratio<Int> outer_scale_exact_;
                /* (b/b')^q -- until c++26 only allow q=0 or q=1/2 */
                double  outer_scale_sq_;
            };

            template <typename Int>
            struct bpu2_rescale_result {
                constexpr bpu2_rescale_result(const bpu2<Int> & bpu_rescaled,
                                              const ratio::ratio<Int> & outer_scale_exact,
                                              double outer_scale_sq)
                    : bpu_rescaled_{bpu_rescaled},
                      outer_scale_exact_{outer_scale_exact},
                      outer_scale_sq_{outer_scale_sq}
                    {}

                /* (b'.u)^p */
                bpu2<Int> bpu_rescaled_;
                /* (b/b')^p0 */
                ratio::ratio<Int> outer_scale_exact_;
                /* [(b/b')^q]^2 -- until c++26 only allow q=0 or q=1/2 */
                double  outer_scale_sq_;
            };

            template <typename Int>
            constexpr
            bpu2_rescale_result<Int>
            bpu2_rescale(const bpu2<Int> & orig,
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
                    mult_sq = mult.template to<double>();
                } else {
                    // remaining possibilities not supported until c++26
                }

                return bpu2_rescale_result<Int>(bpu2<Int>(orig.native_dim(),
                                                          new_scalefactor,
                                                          orig.power()),
                                                mult.power(orig.power().floor()),
                                                mult_sq);
            }

            template <typename Int>
            constexpr
            outer_scalefactor_result<Int>
            bpu_product_inplace(bpu2<Int> * p_target_bpu,
                                const bpu2<Int> & rhs_bpu_orig)
            {
                assert(rhs_bpu_orig.native_dim() == p_target_bpu->native_dim());

                bpu2_rescale_result<Int> rhs_bpu_rr = bpu2_rescale(rhs_bpu_orig,
                                                                   p_target_bpu->scalefactor());

                *p_target_bpu = bpu2<Int>(p_target_bpu->native_dim(),
                                          p_target_bpu->scalefactor(),
                                          p_target_bpu->power() + rhs_bpu_orig.power());

                return outer_scalefactor_result<Int>(rhs_bpu_rr.outer_scale_exact_,
                                                     rhs_bpu_rr.outer_scale_sq_);
            }

            template <typename Int>
            constexpr
            outer_scalefactor_result<Int>
            bpu_array_product_inplace(natural_unit<Int> * p_target,
                                      const bpu2<Int> & bpu)
            {
                std::size_t i = 0;
                for (; i < p_target->n_bpu(); ++i) {
                    if ((*p_target)[i].native_dim() == bpu.native_dim()) {
                        outer_scalefactor_result<Int> retval = bpu_product_inplace(&((*p_target)[i]), bpu);

                        /* TODO: strip 0 power */

                        return retval;
                    }
                }

                /* control here: i=p_target->n_bpu() */
                p_target->push_back(bpu);

                return outer_scalefactor_result<Int>
                    (ratio::ratio<Int>(1, 1) /*outer_scale_exact*/,
                     1.0 /*outer_scale_sq*/);
            }

            template <typename Int>
            constexpr natural_unit<Int>
            nu_reciprocal(const natural_unit<Int> & nu)
            {
                natural_unit<Int> retval;

                for (std::size_t i = 0; i < nu.n_bpu(); ++i)
                    retval.push_back(nu[i].reciprocal());

                return retval;
            } /*nunit_reciprocal*/

        } /*namespace detail*/

        namespace nu2 {
            constexpr auto nanogram = bpu_array_maker<std::int64_t>::make_bpu_array(make_unit_power<std::int64_t>(bu2::nanogram));
            constexpr auto microgram = bpu_array_maker<std::int64_t>::make_bpu_array(make_unit_power<std::int64_t>(bu2::microgram));
        }
    } /*namespace qty*/
} /*namespace xo*/

/** end natural_unit.hpp **/
