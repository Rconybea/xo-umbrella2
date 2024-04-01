/* @file native_bpu.hpp */

#pragma once

#include "native_bpu_concept.hpp"
#include "basis_unit.hpp"
#include <ratio>

namespace xo {
    namespace obs {
        // ----- native_bpu -----

        /** @class native_bpu

            @brief represent product of a compile-time scale-factor with a rational power of a native unit

            Example:
            native_bpu<universal::time, ratio<1>, ratio<-1,2>> represents unit of 1/sqrt(t)
         **/
        template<
            dim DimId,
            typename InnerScale,
            typename Power = std::ratio<1> >
        struct bpu : basis_unit<DimId, native_unit_for_v<DimId>, InnerScale> {
            static_assert(ratio_concept<Power>);

            /* native_unit provides
             * - scalefactor_type --> std::ratio
             * - c_native_dim :: dim
             * - c_native_unit :: native_unit
             */

            using power_type = Power;

            static const int c_num = Power::num;
            static const int c_den = Power::den;
        };

        /** @class bpu_assemble_abbrev
         *
         *  @brief generate abbreviation literal.
         *
         *  Abbreviation literal ignores outer scale factor;
         *  (outer scale factor should be multiplied by run-time scale when printing a quantity)
         *
         *  Separate template from native_bpu so that abbrev can independently be specialized
         **/
        template < dim dim_id,
                   typename InnerScale,
                   typename Power = std::ratio<1> >
        constexpr auto bpu_assemble_abbrev_helper() {
            static_assert(ratio_concept<Power>);

            return stringliteral_concat(units::scaled_native_unit_abbrev_v<dim_id, InnerScale>.value_,
                                        stringliteral_from_exponent<Power>().value_);
        };

        /** Expect:
         *  - BPU is a native_bpu type:
         *    - BPU::scalefactor_type = std::ratio<..>
         *    - BPU::c_native_dim :: dim
         *    - BPU::power_type = std::ratio<..>
         *    - BPU::c_num :: int
         *    - BPU::c_den :: int
         **/
        template < typename BPU >
        constexpr auto bpu_assemble_abbrev() {
            static_assert(native_bpu_concept<BPU>);

            return bpu_assemble_abbrev_helper< BPU::c_native_dim,
                                               typename BPU::scalefactor_type,
                                               typename BPU::power_type >();
        };

        // ----- bpu_rescale -----

        /**
         *  Part I
         *  ------
         *  We have B satisfying native_bpu_concept:
         *  B represents a basis-power-unit
         *          p
         *     (b.u)
         *
         *  with
         *    b = B::scalefactor_type, e.g. 60 for a 1-minute unit
         *    u = B::dim, e.g. 1second for time
         *    p = B::power_type
         *
         *  We want to construct something with similar form:
         *
         *             p
         *    a'.(b'.u)
         *
         *  representing the same dimensioned unit,
         *  i.e.
         *          p            p'
         *     (b.u)  = a'.(b'.u)
         *
         *  with NewInnerScale -> b'
         *
         *          p         p        p            p
         *     (b.u)  = (b/b') . (b'.u)  = a'.(b'.u)
         *
         *                 p
         * with a' = (b/b')
         *
         * For example: if we have B(b=60,u=time,p=2),  NewInnerScale=1:
         * then we want a'=3600, B'(b=1,u=time,p=2)
         *
         * Result represented with
         *   bpu_rescale<B,NewInnerScale>::outer_scalefactor_type -> 'a
         *   bpu_rescale<B,NewInnerScale>::native_bpu_type -> B'
         *
         *  Part II
         *  -------
         *  Want ability to rescale when p is a non-integer rational.
         *  In that case a' = (b/b')^p won't in general be exactly-representable,
         *  so we are forced to accept some loss of precision.
         *
         *  Want to write:
         *    p as p' + q' with:
         *       p' = integer part of p
         *       q' = fractional part of p
         *  Then we can write
         *    a' as c'.d' with:
         *       c' = (b/b')^p'   [exactly represented]
         *       d' = (b/b')^q'   [floating point]
         **/
        template <typename B,
                  typename NewInnerScale>
        struct bpu_rescale {
            static_assert(native_bpu_concept<B>);
            static_assert(ratio_concept<NewInnerScale>);

            /* TODO:
             * - native_unit::c_scale -> std::ratio,  call it c_inner_scalefactor
             * - ++ native_bpu::c_outer_scalefactor,  will be a std::ratio
             */

            /* b/b' */
            using _t1_type = std::ratio_divide
                < typename B::scalefactor_type, NewInnerScale >;

            /* p' */
            using p1_type = ratio_floor_t<typename B::power_type>;
            /* q' */
            using q1_type = ratio_frac_t<typename B::power_type>;

            /** require p must be integral **/
            static_assert(p1_type::den == 1);

            /* note: constexpr from c++26,  but already present in earlier gcc */
            static constexpr double c_outer_scalefactor_inexact = ::pow(from_ratio<double, _t1_type>(),
                                                                        from_ratio<double, q1_type>());

            /**            p
             *  a' = (b/b')
             **/
            using outer_scalefactor_type = ratio_power_t< _t1_type, p1_type::num >;

            /**
             *          p
             *    (b'.u)
             **/
            using native_bpu_type = bpu < B::c_native_dim,
                                          NewInnerScale,
                                          typename B::power_type >;
        };

        // ----- bpu_invert -----

        /** invert a native bpu:  create type for space 1/B **/
        template <typename B>
        struct bpu_invert {
            using type = bpu <
                B::c_native_dim,
                typename B::scalefactor_type,
                std::ratio_multiply<std::ratio<-1>, typename B::power_type>
                >;
        };

        // ----- bpu_product -----

        /** Suppose we have two native_bpu's {B1, B2} that scale the same native basis unit u.
         *  B1,B2 may be using different units {b1,b2} for u
         *
         *                            p1
         *    B1 (b1, u, p1)  = (b1.u)
         *
         *                            p2
         *    B2 (b2, u, p2)  = (b2.u)
         *
         *  we want a representation in similar form:
         *
         *                                    p'
         *    a' . B' (b', u, p')  = a'.(b'.u)
         *
         *  for the product (B1 x B2),  i.e.
         *
         *             p'         p1      p2
         *    a'.(b'.u)   = (b1.u)  (b2.u)
         *
         *  We can use bpu_rescale to rewrite B2 in the form
         *
         *                        p2
         *    B2' = (c'.d').(b1.u)
         *
         *  where c' is exact, d' is inexact.
         *  (note however d' will be exactly 1.0 whenever p2 is integral)
         *
         *  so we have
         *
         *                      p1              p2
         *    (B1 x B2) = (b1.u)  (c'.d').(b1.u)
         *
         *                              p1+p2
         *              = (c'.d').(b1.u)
         *
         **/
        template < typename B1, typename B2 >
        struct bpu_product {
            static_assert(native_bpu_concept<B1>);
            static_assert(native_bpu_concept<B2>);
            static_assert(B1::c_native_dim == B2::c_native_dim);

            /* c'.d'.B2' = c'.d'.(b1.u)^p2
             *
             *  _b2p_rescaled_type::native_bpu_type          -> B2' (b1, u, p2)   [same basis scalefactor as B1]
             *  _b2p_rescaled_type::outer_scalefactor_type   -> c'                [exact factor]
             *  _b2p_rescaled_type::c_outer_scalefactor_type -> d'                [inexact factor, from fractional powers]
             */
            using _b2p_rescaled_type = bpu_rescale<B2,
                                                   typename B1::scalefactor_type>;
            /* (b1.u)^p2 */
            using _b2p_sf_bpu_type = _b2p_rescaled_type::native_bpu_type;

            /* p1+p2 */
            using _p_type = std::ratio_add<
                typename B1::power_type,
                typename B2::power_type
                >;

            /* c' */
            using outer_scalefactor_type = _b2p_rescaled_type::outer_scalefactor_type;
            /* d' */
            static constexpr double c_outer_scalefactor_inexact = _b2p_rescaled_type::c_outer_scalefactor_inexact;

            /* (b1.u)^(p1+p2) */
            using native_bpu_type = bpu <
                B1::c_native_dim,
                typename B1::scalefactor_type,
                _p_type /*Power*/ >;
        };

    } /*namespace obs*/
} /*namespace xo*/

/* end native_bpu.hpp */
