/* @file dimension_impl.hpp */

#pragma once

#include "dimension_concept.hpp"
#include "native_bpu.hpp"
#include "dim_util.hpp"
#include "ratio_util.hpp"
//#include <json/value.h>
#include <ratio>
#include <algorithm>
#include <numeric>
#include <string_view>

namespace xo {
    /* TODO:
     * - bpu_list -> bpu_node
     */

    namespace unit {
        // ----- lookup_bpu -----

        /**
         *  Select from dimension_impl by known index value
         *
         *  Example:
         *    using t1 = native_bpu<dim::currency, std::ratio<1>, std::ratio<1,1>>;
         *    using t2 = native_bpu<dim::time,     std::ratio<60>, std::ratio<-1,2>>;
         *    using dim = dimension_impl<t1,t2>
         *
         * then
         *    lookup_bpu<dim,0> --> t1
         *    lookup_bpu<dim,1> --> t2
         **/
        template <typename Dim, int Index>
        struct lookup_bpu  {
            using power_unit_type = lookup_bpu<typename Dim::rest_type, Index-1>::power_unit_type;
        };

        template <typename Dim>
        struct lookup_bpu<Dim, 0> {
            using power_unit_type = Dim::front_type;
        };

        // ----- di_find_bpu -----

        /**
         *  @brief Select from dimension_impl by native_dim_id
         *
         *  Example:
         *    using t1 = native_bpu<dim::time,     std::ratio<60>, std::ratio<-2>>;
         *    using t2 = native_bpu<dim::currency, std::ratio<1>, std::ratio<1>>;
         *    using di = dimension_impl<t1,t2>;
         *
         * then
         *    di_find_bpu<dim::time> -> t1
         *    di_find_bpu<dim::currency> -> t2
         *    di_find_bpu<dim::mass> -> native_bpu {dim::mass, std::ratio<1>, std::ratio<0>}
         **/
        template <typename BpuList, dim BasisDim>
        struct di_find_bpu;

        /**
         *  @brief Aux template helper for di_find_bpu<..>
         **/
        template <typename Front, typename Rest, dim BasisDim, bool MatchesFront = (Front::c_native_dim == BasisDim)>
        struct di_find_bpu_aux;

        /** specialization for non-empty BpuList **/
        template <typename BpuList, dim BasisDim>
        struct di_find_bpu {
            using type = di_find_bpu_aux<typename BpuList::front_type, typename BpuList::rest_type, BasisDim>::type;
        };

        /** specialization for empty BpuList **/
        template <dim BasisDim>
        struct di_find_bpu<void, BasisDim> {
            using type = bpu<BasisDim, std::ratio<1>, std::ratio<0>>;
        };

        template <typename Front, typename Rest, dim BasisDim>
        struct di_find_bpu_aux<Front, Rest, BasisDim, /*MatchesFront*/ true> {
            using type = Front;
        };

        template <typename Front, typename Rest, dim BasisDim>
        struct di_find_bpu_aux<Front, Rest, BasisDim, /*MatchesFront*/ false> {
            using type = di_find_bpu<Rest, BasisDim>::type;
        };

        // ----------------------------------------------------------------

        /**
         *  Promise:
         *  - bpu_list::front_type
         *  - bpu_list::rest_type
         *  - bpu_node_concept<bpulist<P,D>>
         **/
        template <typename P, typename D = void>
        struct bpu_node;

        // ----------------------------------------------------------------

        template <typename Front,
                  typename Rest>
        constexpr bool FrontHasZeroPower = (Front::power_type::num == 0); //std::ratio_equal_v< typename Front::power_type, std::ratio<0,1> >;

        template <typename Front,
                  typename Rest,
                  bool FHZP = FrontHasZeroPower<Front, Rest>>
        struct bpu_smart_cons;

        template <typename Front,
                  typename Rest>
        struct bpu_smart_cons<Front, Rest, /*FrontHasZeroPower*/ true> {
            using type = Rest;
        };

        template <typename Front,
                  typename Rest>
        struct bpu_smart_cons<Front, Rest, /*FrontHasZeroPower*/ false> {
            using type = bpu_node<Front, Rest>;
        };

        template <typename Front, typename Rest>
        using bpu_smart_cons_t = bpu_smart_cons<Front, Rest>::type;

        // ----------------------------------------------------------------

        /** @class bwp

            @brief represent (compile-time) result of search in a bpu_list<> type

            short for (basis-with-native-power-unit)
         **/
        template <int index_arg, dim basis_arg>
        struct bwp {
            static constexpr int c_index = index_arg;
            static constexpr dim c_basis = basis_arg;
        };

        template <typename T>
        using bwp_incr_pos_type = bwp<T::c_index + 1, T::c_basis>;

        // ----- lo_basis_with_pos_type -----

        template < typename BasisWithPos1, typename BasisWithPos2>
        using lo_basis_with_pos_type = std::conditional_t<(BasisWithPos1::c_basis < BasisWithPos2::c_basis),
                                                          BasisWithPos1, BasisWithPos2>;

        // ----- native_lo_bwp_of -----

        /* helper for canonically-ordering native dimension power-units */
        template <typename Dim>
        struct native_lo_bwp_of {
            using _bwp_front = bwp<0, Dim::front_type::c_native_dim>;
            using _pu_rest  = native_lo_bwp_of<typename Dim::rest_type>;
            using _bwp_rest = typename _pu_rest::bwp_type;

            using bwp_type = lo_basis_with_pos_type<_bwp_front,
                                                    bwp_incr_pos_type<_bwp_rest>>;
        };

        template <typename P0>
        struct native_lo_bwp_of<bpu_node<P0>> {
            using bwp_type = bwp<0, P0::c_native_dim>;
        };

        // ----- without_elt -----

        template <typename Dim, int Index>
        struct without_elt {
            using _without_rest_type = typename without_elt<typename Dim::rest_type, Index - 1>::dim_type;

            using dim_type = bpu_node< typename Dim::front_type, _without_rest_type >;
        };

        template <typename Dim>
        struct without_elt<Dim, 0> {
            using dim_type = typename Dim::rest_type;
        };

        // ----- bpu_node -----

        /** Represents the cartesian product of a list of 'native basis power units';
         *  represents something with dimensions
         *
         *  Expect:
         *  - P isa native_bpu type
         *  - D satisfies bpu_list_concept
         **/
        template <typename P, typename D>
        struct bpu_node {
            static_assert(native_bpu_concept<P>);
            static_assert(bpu_list_concept<D>);

            /** For example:
             *    using b1 = basis_power_unit<dim::currency, std::ratio<1, 1>>;
             *    using b2 = basis_power_unit<dim::time, std::ratio<-1, 2>>;
             *    using foo = dimension_impl<b1,dimension_impl<b2>>;
             *
             *  then
             *    foo::lookup_bpu<0> -> b1
             *    foo::lookup_bpu<1> -> b2
             *    foo::lookup_bpu<2> -> not defined
             **/
            using front_type = P;
            using rest_type = D;

            static constexpr std::uint32_t n_dimension = rest_type::n_dimension + 1;
        };

        /** @class dimension

            @brief represent a composite dimension
        **/
        template <typename P0>
        struct bpu_node<P0, void> {
            static_assert(native_bpu_concept<P0>);
            static_assert(bpu_list_concept<void>);

            using front_type = P0;
            using rest_type = void;

            /** For example:
             *    using b1 = basis_power_unit<dim::time, std::ratio<-1, 2>>;
             *    using foo = dimension_impl<b1>;
             *  then
             *    foo::lookup_bpu<0> --> b1
             *    foo::lookup_bpu<1> --> not defined
             **/

            /** number of dimensions represented by this struct **/
            static constexpr std::uint32_t n_dimension = 1;
        };

        // ----- di_replace_basis_scale -----

        /**
         *  @brief Replace BpuList member with matching BasisDim, preserving everything except (inner) scalefactor
         **/
        template <typename BpuList, typename NewBpu>
        struct di_replace_basis_scale;

        template <typename Front, typename Rest, typename NewBpu, bool MatchesFront = (Front::c_native_dim == NewBpu::c_native_dim)>
        struct di_replace_basis_scale_aux;

        /** specialization for non-empty BpuList **/
        template <typename BpuList, typename NewBpu>
        struct di_replace_basis_scale {
            using type = di_replace_basis_scale_aux<typename BpuList::front_type,
                                                    typename BpuList::rest_type,
                                                    NewBpu>::type;
        };

        /** specialization for empty BpuList -- error not found **/
        template <typename NewBpu>
        struct di_replace_basis_scale<void, NewBpu> {};

        /** specialization for matching front **/
        template <typename Front, typename Rest, typename NewBpu>
        struct di_replace_basis_scale_aux<Front, Rest, NewBpu, /*MatchesFront*/ true> {
            using _replace_bpu_type = bpu<Front::c_native_dim,
                typename NewBpu::scalefactor_type,
                typename Front::power_type>;

            static_assert(native_bpu_concept<_replace_bpu_type>);

            /* NewBpu replaces Front */
            using type = bpu_node<_replace_bpu_type, Rest>;
        };

        template <typename Front, typename Rest, typename NewBpu>
        struct di_replace_basis_scale_aux<Front, Rest, NewBpu, /*MatchesFront*/ false> {
            /* keep Front, replace NewBpu in rest */
            using type = bpu_node<Front, typename di_replace_basis_scale<Rest, NewBpu>::type>;
        };

        // ----- bpu_cartesian_product -----

        /** Require:
         *  - B isa native_bpu type
         *  - DI_Front is a native_bpu type
         *  - DI_Rest is a dimension_impl type
         *
         *  Promise:
         *  - type isa dimension_impl type
         **/
        template < typename B,
                   typename DI_Front,
                   typename DI_Rest,
                   bool MatchesFront = (B::c_native_dim == DI_Front::c_native_dim) >
        struct bpu_cartesian_product_helper;

        /** require:
         * - B isa native_bpu type
         * - DI isa (bpu_list | void) type
         **/
        template < typename B, typename DI >
        struct bpu_cartesian_product {
            static_assert(native_bpu_concept<B>);
            static_assert(bpu_list_concept<DI>);

            using _tmp = bpu_cartesian_product_helper<B,
                                                      typename DI::front_type,
                                                      typename DI::rest_type>;

            using outer_scalefactor_type = typename _tmp::outer_scalefactor_type;
            static constexpr double c_outer_scalefactor_inexact = _tmp::c_outer_scalefactor_inexact;

            using bpu_list_type = typename _tmp::bpu_list_type;

            static_assert(ratio_concept<outer_scalefactor_type>);
            static_assert(bpu_list_concept<bpu_list_type>);
        };

        /** Reminder: void represents the 'no dimension' of a dimensionless quantity **/
        template < typename B >
        struct bpu_cartesian_product<B, void> {
            using outer_scalefactor_type = std::ratio<1>;
            static constexpr double c_outer_scalefactor_inexact = 1.0;

            using bpu_list_type = bpu_node<B, void>;

            static_assert(ratio_concept<outer_scalefactor_type>);
            static_assert(bpu_list_concept<bpu_list_type>);
        };

        /* specialize for matching front */
        template <typename B, typename DI_Front, typename DI_Rest>
        struct bpu_cartesian_product_helper<B, DI_Front, DI_Rest, /*MatchesFront*/ true> {
            static_assert(native_bpu_concept<B>);
            static_assert(native_bpu_concept<DI_Front>);
            static_assert(bpu_list_concept<DI_Rest>);

            /* _mult_type may have zero exponent (power_type);
             *  in that case bpu_smart_cons will collapse to DI_Rest
             */
            using _front_mult_type = bpu_product<B, DI_Front>;

            using _front_type = typename _front_mult_type::native_bpu_type;
            using _rest_type = DI_Rest;

            using outer_scalefactor_type = typename _front_mult_type::outer_scalefactor_type;
            static constexpr double c_outer_scalefactor_inexact = _front_mult_type::c_outer_scalefactor_inexact;

            using bpu_list_type = bpu_smart_cons_t<_front_type, DI_Rest>;

            static_assert(ratio_concept<outer_scalefactor_type>);
            static_assert(bpu_list_concept<bpu_list_type>);
        };

        /* specialize for not-matching-front */
        template <typename B, typename DI_Front, typename DI_Rest>
        struct bpu_cartesian_product_helper<B, DI_Front, DI_Rest, /*MatchesFront*/ false> {
            static_assert(native_bpu_concept<B>);
            static_assert(native_bpu_concept<DI_Front>);
            static_assert(bpu_list_concept<DI_Rest>);

            using _rest_mult_type = bpu_cartesian_product< B, DI_Rest >;

            using _front_type = DI_Front;
            using _rest_type = typename _rest_mult_type::bpu_list_type;

            using outer_scalefactor_type = typename _rest_mult_type::outer_scalefactor_type;
            static constexpr double c_outer_scalefactor_inexact = _rest_mult_type::c_outer_scalefactor_inexact;

            using bpu_list_type = bpu_node<DI_Front, _rest_type>;

            static_assert(ratio_concept<outer_scalefactor_type>);
            static_assert(bpu_list_concept<bpu_list_type>);
        };

        // ----- di_cartesian_product -----

        template < typename D1, typename D2 > struct di_cartesian_product;

        // ----- bpu_cartesian_product1 -----

        template < typename B1, typename R1, typename D2 >
        struct di_cartesian_product1 {
            static_assert(native_bpu_concept<B1>);
            static_assert(bpu_list_concept<R1>);
            static_assert(bpu_list_concept<D2>);

            using _tmp1_mult_type = bpu_cartesian_product<B1, D2>;
            using _tmp1_scalefactor_type = _tmp1_mult_type::outer_scalefactor_type;
            using _tmp1_bpu_list_type = _tmp1_mult_type::bpu_list_type;

            using _tmp2_mult_type = di_cartesian_product<R1, _tmp1_bpu_list_type>;
            using _tmp2_scalefactor_type = _tmp2_mult_type::outer_scalefactor_type;
            using _tmp2_bpu_list_type = _tmp2_mult_type::bpu_list_type;

            using outer_scalefactor_type = std::ratio_multiply<
                _tmp1_scalefactor_type,
                _tmp2_scalefactor_type>;
            static constexpr double c_outer_scalefactor_inexact = (_tmp1_mult_type::c_outer_scalefactor_inexact
                                                                   * _tmp2_mult_type::c_outer_scalefactor_inexact);

            using bpu_list_type = _tmp2_bpu_list_type;

            static_assert(ratio_concept<outer_scalefactor_type>);
            static_assert(bpu_list_concept<bpu_list_type>);
        };

        template < typename B1, typename D2 >
        struct di_cartesian_product1<B1, void, D2> {
            static_assert(native_bpu_concept<B1>);
            static_assert(bpu_list_concept<D2>);

            using _tmp_mult_type = bpu_cartesian_product<B1, D2>;

            using outer_scalefactor_type = _tmp_mult_type::outer_scalefactor_type;
            static constexpr double c_outer_scalefactor_inexact = _tmp_mult_type::c_outer_scalefactor_inexact;

            using bpu_list_type = _tmp_mult_type::bpu_list_type;
        };

        // ----- di_invert -----

        /* note: rescaling never required here,
         *       since not combining basis dimensions.
         */
        template <typename BpuList>
        struct di_invert;

        template <>
        struct di_invert<void> {
            using type = void;
        };

        template <typename BpuList>
        struct di_invert {
            using type = bpu_node<
                typename bpu_invert<typename BpuList::front_type>::type,
                typename di_invert<typename BpuList::rest_type>::type
                >;
        };

        // ----- di_cartesian_product -----

        template < typename D1, typename D2 >
        struct di_cartesian_product {
            static_assert(bpu_list_concept<D1>);
            static_assert(bpu_list_concept<D2>);

            using _mult_type = di_cartesian_product1<
                typename D1::front_type,
                typename D1::rest_type,
                D2>;

            using outer_scalefactor_type = _mult_type::outer_scalefactor_type;
            static constexpr double c_outer_scalefactor_inexact = _mult_type::c_outer_scalefactor_inexact;

            using bpu_list_type = _mult_type::bpu_list_type;

            static_assert(ratio_concept<outer_scalefactor_type>);
            static_assert(bpu_list_concept<bpu_list_type>);
        };

        template < typename D2 >
        struct di_cartesian_product< void, D2 > {
            static_assert(bpu_list_concept<D2>);

            using outer_scalefactor_type = std::ratio<1>;
            static constexpr double c_outer_scalefactor_inexact = 1.0;
            using bpu_list_type = D2;
        };

        template <typename D1 >
        struct di_cartesian_product< D1, void > {
            static_assert(bpu_list_concept<D1>);

            using outer_scalefactor_type = std::ratio<1>;
            static constexpr double c_outer_scalefactor_inexact = 1.0;
            using bpu_list_type = D1;
        };

        // ----- di_assemble_abbrev -----

        /* reminder: can't partially specialize a template function -> need struct wrapper */
        template < typename DI >
        struct di_assemble_abbrev;

        /** Expect:
         *  - P isa native_bpu type
         *    - P::power_type = std::ratio<..>
         *    - P::c_native_dim  :: dim
         *    - P::c_num :: int
         *    - P::c_den :: int
         *  - D isa dimension_impl type
         *    - D::front_type = native_bpu<..>
         *    - D::rest_type = dimension_impl<..>
         *    - D::n_dimension :: int
         **/
        template <typename P, typename D>
        struct di_assemble_abbrev_helper {
            static_assert(native_bpu_concept<P>);
            static_assert(bpu_list_concept<D>);

            static constexpr auto _prefix = bpu_assemble_abbrev<P>();
            static constexpr auto _suffix = di_assemble_abbrev<D>::value;

            static constexpr auto value = stringliteral_concat(_prefix.value_,
                                                               ".",
                                                               _suffix.value_);
        };

        template <typename P>
        struct di_assemble_abbrev_helper<P, void> {
            static constexpr auto value = bpu_assemble_abbrev<P>();
        };

        template < typename DI >
        struct di_assemble_abbrev {
            static_assert(bpu_list_concept<DI>);

            using _helper_type = di_assemble_abbrev_helper <typename DI::front_type, typename DI::rest_type>;

            static constexpr auto value = _helper_type::value;
        };

        template <>
        struct di_assemble_abbrev<void> {
            static constexpr auto value = stringliteral("");
        };

        // ----- canonical_impl -----

        template <typename D>
        struct canonical_impl {
            /*
             * bwp_front::c_index
             * bwp_front::c_native_dim
             */
            using _bwp_front = native_lo_bwp_of<D>::bwp_type;

            using _front_type = typename lookup_bpu<D, _bwp_front::c_index>::power_unit_type;
            using _rest0_type = typename without_elt<D, _bwp_front::c_index>::dim_type;
            using _rest_type = canonical_impl<_rest0_type>::dim_type;

            using dim_type = bpu_node<_front_type, _rest_type>;
        };

        /** compute canonical renumbering of a dimension
         **/
        template <>
        struct canonical_impl<void> {
            using dim_type = void;
        };

        template<typename D>
        using canonical_t = canonical_impl<D>::dim_type;

    } /*namespace unit*/
} /*namespace xo*/

/* end dimension_impl.hpp */
