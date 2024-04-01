/* @file dimension.hpp */

#pragma once

#include "dimension_impl.hpp"

namespace xo {
    namespace obs {
        // ----- wrap_unit -----

        template <typename Scalefactor, typename BpuList>
        struct wrap_unit {
            static_assert(ratio_concept<Scalefactor>);
            static_assert(bpu_list_concept<BpuList>);

            //using _norm_type = bpu_normalize<BpuList>;

            using scalefactor_type = Scalefactor;
            using dim_type = BpuList;

            /* canon_type just orders dimensions by increasing native_dim_id */
            using canon_type = canonical_impl<dim_type>::dim_type;

            static_assert(bpu_list_concept<canon_type>);
        };

        // ----- normalize_unit -----

        template <typename Unit>
        struct normalize_unit {
            static_assert(unit_concept<Unit>);

            using type = wrap_unit<std::ratio<1>, typename Unit::dim_type>;
        };

        template <typename Unit>
        using normalize_unit_t = normalize_unit<Unit>::type;

        // ----- dimensionless_v -----

        template <typename Unit>
        constexpr auto dimensionless_v = std::same_as<typename Unit::dim_type, void>;

        // ----- unit_find_bpu -----

        /** @brief find basis-power-unit matching native_dim_id
         *
         *  Constructs dimensionless native_bpu if no match
         **/
        template <typename U, dim BasisDim>
        struct unit_find_bpu {
            using type = di_find_bpu<typename U::dim_type, BasisDim>::type;
        };

        template <typename U, dim BasisDim>
        using unit_find_bpu_t = unit_find_bpu<U, BasisDim>::type;

        // ----- unit_abbrev_v -----

        /** @brief canonical stringliteral abbreviation for dimension D. **/
        template <typename U>
        constexpr auto unit_abbrev_v = di_assemble_abbrev<typename U::canon_type>::value;

        // ----- unit_invert -----

        template <typename U>
        struct unit_invert {
            static_assert(unit_concept<U>);

            using _sf = std::ratio_divide<std::ratio<1>, typename U::scalefactor_type>;
            using _di = di_invert< typename U::dim_type >::type;

            using type = wrap_unit< _sf, _di >;

            static_assert(unit_concept<type>);
        };

        template <typename U>
        using unit_invert_t = unit_invert<U>::type;

        // ----- unit_cartesian_product -----

        template <typename U1, typename U2>
        struct unit_cartesian_product {
            /* when a basis dimension (represented by a bpu<..>::c_native_dim)
             * is present in both {U1, U2},  we need to pick a common unit
             * (represented by bpu<..>::scalefactor_type).
             *
             * scale factors from such conversions are collected in:
             * 1a. _mult_type::outer_scalefactor_type     (compile-time exact representation using std::ratio)
             * 1b. _mult_type::outer_scalefactor_inexact  (compile-time constexpr)
             */

            static_assert(unit_concept<U1>);
            static_assert(unit_concept<U2>);

            /* _mult_type -> describes product dimension */
            using _mult_type = di_cartesian_product<
                typename U1::dim_type,
                typename U2::dim_type>;

            /* compile-time exact scalefactor for product dimension
             * (distilled from any forced rescaling)
             */
            using _mult_sf_type = _mult_type::outer_scalefactor_type;
            /* bpulist specifying basis factors (possibly to rational powers) in product dimension */
            using _mult_di_type = _mult_type::bpu_list_type;

            /* note: inexact scalefactor doesn't come up here.
             *       It's not present in unit types,  only appears as byproduct
             *       of products/ratios of units
             */
            using _sf1_type = typename std::ratio_multiply<
                typename U1::scalefactor_type,
                typename U2::scalefactor_type>::type;

            using _sf_type = typename std::ratio_multiply<_mult_sf_type, _sf1_type>::type;

            /* note: we can compute inexact scale factor,
             *       but can't make it a template argument
             */
            using exact_unit_type = wrap_unit< _sf_type, _mult_di_type >;

            static constexpr double c_scalefactor_inexact = _mult_type::c_outer_scalefactor_inexact;

            static_assert(unit_concept<exact_unit_type>);
        };

        /* WARNING: omits inexact scalefactor */
        template <typename U1, typename U2>
        using unit_cartesian_product_t = unit_cartesian_product<U1, U2>::exact_unit_type;

        // ----- unit_divide -----

        template <typename U1, typename U2>
        struct unit_divide {
            static_assert(unit_concept<U1>);
            static_assert(unit_concept<U2>);

            using _mult_type = unit_cartesian_product<U1, unit_invert_t<U2>>;
            using exact_unit_type = _mult_type::exact_unit_type;

            static constexpr double c_scalefactor_inexact = _mult_type::c_scalefactor_inexact;
        };

        /* WARNING: omits inexact scalefactor */
        template <typename U1, typename U2>
        using unit_divide_t = unit_divide<U1, U2>::exact_unit_type;

        // ----- same_dimension -----

        /* true iff U1 and U2 represent the same dimension,  i.e. differ only in dimensionless scaling factor
         *
         * To verify scale also, use same_unit<U1,U2> instead
         */
        template <typename U1, typename U2>
        struct same_dimension {
            static_assert(unit_concept<U1>);
            static_assert(unit_concept<U2>);

            using _unit_ratio_type = typename unit_cartesian_product<U1, unit_invert_t<U2>>::exact_unit_type;

            static_assert(std::same_as<typename _unit_ratio_type::dim_type, void>);

            static constexpr bool value = std::same_as<typename _unit_ratio_type::dim_type, void>;
        };

        template <typename U1, typename U2>
        constexpr bool same_dimension_v = same_dimension<U1, U2>::value;

        // ----- same_unit -----

        template <typename U1, typename U2>
        struct same_unit {
            static_assert(unit_concept<U1>);
            static_assert(unit_concept<U2>);

            using _unit_ratio_type = unit_cartesian_product<U1, unit_invert_t<U2>>;
            using _unit_exact_type = typename _unit_ratio_type::exact_unit_type;
            using _unit_scalefactor_type = _unit_exact_type::scalefactor_type;
            static constexpr double c_unit_ratio_inexact = _unit_ratio_type::c_scalefactor_inexact;

            static_assert(std::same_as<_unit_scalefactor_type, std::ratio<1>>);
            static_assert(std::same_as<typename _unit_exact_type::dim_type, void>);

            static constexpr bool value = (std::same_as<_unit_scalefactor_type, std::ratio<1>>
                                           && (c_unit_ratio_inexact == 1.0)
                                           && std::same_as<typename _unit_exact_type::dim_type, void>);
        };

        template <typename U1, typename U2>
        constexpr bool same_unit_v = same_unit<U1, U2>::value;

        // ----- unit_conversion_factor -----

        template <typename U1, typename U2>
        struct unit_conversion_factor {
            static_assert(same_dimension_v<U1, U2>);

            using _unit_ratio_type = typename unit_cartesian_product<U1, unit_invert_t<U2>>::exact_unit_type;
            using type = _unit_ratio_type::scalefactor_type;
            static constexpr double c_scalefactor_inexact = _unit_ratio_type::c_scalefactor_inexact;
        };

        /** conversion factor from U1 to U2:
         *    U1 = x.U2
         *  with:
         *    x = R::num / R::den
         *    R = unit_conversion_factor_t<U1,U2>
         *
         *  WARNING: omits inexact scalefactor unit_conversion_factor<U1,U2>::c_scalefactor_inexact
         **/
        template <typename U1, typename U2>
        using unit_conversion_factor_t = unit_conversion_factor<U1, U2>::type;

        // ----- units -----

        namespace units {
            /* computing abbreviations:
             *  - unit_abbrev_v<Unit> :: stringliteral<...>
             *  - unit_abbrev_v<Unit>.c_str() :: const char *
             *
             * relies on
             *  - di_assemble_abbrev, di_assemble_abbrev_helper  [dimension_impl.hpp]
             *
             *  - bpu_assemble_abbrev<native_bpu>()              [native_bpu.hpp]
             *  - bpu_assemble_abbrev_helper< native_bpu::c_native_dim,
             *                                native_bpu::scalefactor_type,
             *                                native_bpu::power_type >
             *     -> stringliteral
             *
             *    + can specialize for specific combinations
             *
             *  - native_unit_abbrev_helper< native_bpu::c_native_dim,
             *                               native_bpu::power_type >
             */

            // ----- weight -----

            using milligram  = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::mass,
                                                               std::ratio<1, 1000>> > >;

            template <>
            struct scaled_native_unit_abbrev<dim::mass, std::ratio<1, 1000>> {
                static constexpr auto value = stringliteral("mg");
            };

            using gram       = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::mass,
                                                               std::ratio<1>> > >;
            using kilogram   = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::mass,
                                                               std::ratio<1000>> > >;

            template <>
            struct scaled_native_unit_abbrev<dim::mass, std::ratio<1000>> {
                static constexpr auto value = stringliteral("kg");
            };

            // ----- distance -----

            using millimeter = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::distance,
                                                               std::ratio<1,1000>> > >;
            template <>
            struct scaled_native_unit_abbrev<dim::distance, std::ratio<1,1000>> {
                static constexpr auto value = stringliteral("mm");
            };

            using meter      = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::time,
                                                               std::ratio<1>> > >;

            using kilometer  = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::time,
                                                               std::ratio<1000>> > >;
            template <>
            struct scaled_native_unit_abbrev<dim::distance, std::ratio<1000>> {
                static constexpr auto value = stringliteral("km");
            };

            // ----- time -----

            using nanosecond = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::time,
                                                               std::ratio<1, 1000000000>> > >;

            template <>
            struct scaled_native_unit_abbrev<dim::time, std::ratio<1,1000000000>> {
                static constexpr auto value = stringliteral("ns");
            };

            using microsecond = wrap_unit< std::ratio<1>,
                                           bpu_node< bpu<dim::time,
                                                                std::ratio<1, 1000000>> > >;

            template <>
            struct scaled_native_unit_abbrev<dim::time, std::ratio<1,1000000>> {
                static constexpr auto value = stringliteral("us");
            };

            using millisecond = wrap_unit< std::ratio<1>, bpu_node< bpu<dim::time,
                                                                               std::ratio<1,1000>> > >;

            template <>
            struct scaled_native_unit_abbrev<dim::time, std::ratio<1,1000>> {
                static constexpr auto value = stringliteral("ms");
            };

            using second     = wrap_unit< std::ratio<1>, bpu_node< bpu<dim::time,
                                                                              std::ratio<1>> > >;
            using minute     = wrap_unit< std::ratio<1>, bpu_node< bpu<dim::time,
                                                                              std::ratio<60>> > >;
            template <>
            struct scaled_native_unit_abbrev<dim::time, std::ratio<60>> {
                static constexpr auto value = stringliteral("min");
            };

            using hour       = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::time,
                                                        std::ratio<3600>> > >;

            template <>
            struct scaled_native_unit_abbrev<dim::time, std::ratio<3600>> {
                static constexpr auto value = stringliteral("hr");
            };

            using day       = wrap_unit< std::ratio<1>,
                                         bpu_node< bpu<dim::time,
                                                       std::ratio<24*3600>> > >;

            template <>
            struct scaled_native_unit_abbrev<dim::time, std::ratio<24*3600>> {
                static constexpr auto value = stringliteral("dy");
            };

            using month     = wrap_unit< std::ratio<1>,
                                         bpu_node< bpu<dim::time,
                                                       std::ratio<30*24*3600>> > >;

            template <>
            struct scaled_native_unit_abbrev<dim::time, std::ratio<30*24*3600>> {
                static constexpr auto value = stringliteral("mo");
            };

            using yr250     = wrap_unit< std::ratio<1>,
                                         bpu_node< bpu<dim::time,
                                                       std::ratio<250*24*3600>> > >;

            template <>
            struct scaled_native_unit_abbrev<dim::time, std::ratio<250*24*3600>> {
                static constexpr auto value = stringliteral("yr250");
            };

            // ------ volatility ------

            /* volatility   in units of 1/sqrt(1day)
             * volatility^2 in units of 1/day
             */
            using volatility_1d = wrap_unit< std::ratio<1>,
                                             bpu_node< bpu<dim::time,
                                                           std::ratio<24*3600>,
                                                           std::ratio<-1,2>> > >;

            /* volatility   in units of 1/sqrt(30day)
             * volatility^2 in units of 1/(30day)
             */
            using volatility_30d = wrap_unit< std::ratio<1>,
                                              bpu_node< bpu<dim::time,
                                                            std::ratio<30*24*3600>,
                                                            std::ratio<-1,2>> > >;

            /* volatility   in units of 1/sqrt(250day)
             * volatility^2 in units of 1/(250day)
             */
            using volatility_250d = wrap_unit< std::ratio<1>,
                                               bpu_node< bpu<dim::time,
                                                             std::ratio<250*24*3600>,
                                                             std::ratio<-1,2>> > >;

            using currency   = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::currency,
                                                               std::ratio<1>> > >;
            using price      = wrap_unit< std::ratio<1>,
                                          bpu_node< bpu<dim::price,
                                                               std::ratio<1>> > >;
        }

    } /*namespace obs*/
} /*namespace xo*/


/* end dimension.hpp */
