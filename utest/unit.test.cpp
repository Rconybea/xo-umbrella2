/* @file dimension.test.cpp */

#include "xo/unit/unit.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/cxxutil/demangle.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
#include <iostream>

namespace xo {
    namespace ut {
        /* compile-time tests */

        using xo::reflect::Reflect;

        using xo::unit::dim;
        using xo::unit::native_unit_abbrev_v;
        using xo::unit::units::scaled_native_unit_abbrev_v;
        //using xo::unit::native_dim_abbrev;
        using xo::unit::stringliteral_compare;
        using xo::unit::literal_size_v;
        using xo::unit::stringliteral_from_digit;
        using xo::unit::stringliteral_from_int_v;
        using xo::unit::stringliteral;
#ifndef __clang__
        using xo::unit::stringliteral_concat;
        using xo::unit::stringliteral_from_ratio;
        using xo::unit::bpu_assemble_abbrev_helper;
        using xo::unit::bpu_assemble_abbrev;
#endif
        using xo::unit::bpu_node;
        using xo::unit::wrap_unit;
        using xo::unit::unit_abbrev_v;
        //using xo::unit::dim_abbrev_v;
        using xo::unit::di_cartesian_product;
        using xo::unit::di_cartesian_product1;
        using xo::unit::unit_cartesian_product_t;
        using xo::unit::bpu_cartesian_product;
        using xo::unit::bpu_cartesian_product_helper;
        using xo::unit::unit_invert_t;
        using xo::unit::units::gram;
        using xo::unit::units::second;
        using xo::print::ccs;

        template <typename T>
        int unused()
        {
            return 1;
        }

        template <typename T1, typename T2>
        constexpr bool unused_same(typename std::enable_if_t<std::is_same<T1, T2>::value, bool> result = true)
        {
            return result;
        }

        TEST_CASE("native_unit_abbrev", "[native_dim_abbrev]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.native_dim_abbrev"));
            //log && log("(A)", xtag("foo", foo));

            /* NOTE: the .value_ expression below will fail to compile if missing specialization for
             *       native_dim_abbrev on native_dim_id::foo;  that's the point :)
             */

            REQUIRE(strcmp(scaled_native_unit_abbrev_v<dim::mass, std::ratio<1>>.value_, "g") == 0);
            REQUIRE(strcmp(scaled_native_unit_abbrev_v<dim::time, std::ratio<1>>.value_, "s") == 0);
            REQUIRE(strcmp(scaled_native_unit_abbrev_v<dim::currency, std::ratio<1>>.value_, "ccy") == 0);
            REQUIRE(strcmp(scaled_native_unit_abbrev_v<dim::price, std::ratio<1>>.value_, "px") == 0);

#ifdef OBSOLETE
            REQUIRE(strcmp(native_dim_abbrev<dim::mass>().value_, "") != 0);
            REQUIRE(strcmp(native_dim_abbrev<dim::time>().value_, "") != 0);
            REQUIRE(strcmp(native_dim_abbrev<dim::currency>().value_, "") != 0);
            REQUIRE(strcmp(native_dim_abbrev<dim::price>().value_, "") != 0);
#endif

            static_assert(stringliteral_compare(stringliteral_from_digit(0), stringliteral("0")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_digit(1), stringliteral("1")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_digit(9), stringliteral("9")) == 0);

            static_assert(literal_size_v<0> == 1);
            static_assert(literal_size_v<10> == 2);
            static_assert(literal_size_v<99> == 2);
            static_assert(literal_size_v<100> == 3);
            static_assert(literal_size_v<999> == 3);

            static_assert(stringliteral_compare(stringliteral_from_int_v<0>(), stringliteral("0")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<9>(), stringliteral("9")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<1, 1, false>(), stringliteral("1")) == 0);


            static_assert(stringliteral_compare(stringliteral_from_int_v<9, 1, false>(), stringliteral("9")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<9>(), stringliteral("9")) == 0);

            /* NOTE: clang16 complains starting here;  gcc is fine */

#ifndef __clang__
            if constexpr (stringliteral_concat("a", "b").size() == 3) {
                REQUIRE(true);
            } else {
                REQUIRE(false);
            }

            static_assert(stringliteral_compare(stringliteral_concat("hello", " ", "world"),
                                                stringliteral("hello world")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<10, 2, false>(), stringliteral("10")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<10>(), stringliteral("10")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<99, 2, false>(), stringliteral("99")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<99>(), stringliteral("99")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<100, 3, false>(), stringliteral("100")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<100>(), stringliteral("100")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<999, 3, false>(), stringliteral("999")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<999>(), stringliteral("999")) == 0);

            //std::cerr << "test=" << stringliteral_from_int_v<-1, 2, true>().value_ << std::endl;

            static_assert(stringliteral_compare(stringliteral_from_int_v<-1, 2, true>(), stringliteral("-1")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<-1>(), stringliteral("-1")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<-9, 2, true>(), stringliteral("-9")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<-9>(), stringliteral("-9")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<-10, 3, true>(), stringliteral("-10")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<-10>(), stringliteral("-10")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<-99, 3, true>(), stringliteral("-99")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<-99>(), stringliteral("-99")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<-100, 4, true>(), stringliteral("-100")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<-100>(), stringliteral("-100")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_int_v<-999, 4, true>(), stringliteral("-999")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_int_v<-999>(), stringliteral("-999")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<2,3>>(), stringliteral("(2/3)")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<4,6>>(), stringliteral("(2/3)")) == 0);

            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<-1>>(), stringliteral("-1")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<-2>>(), stringliteral("-2")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<-6,3>>(), stringliteral("-2")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<-3,2>>(), stringliteral("-(3/2)")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<3,-2>>(), stringliteral("-(3/2)")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<-1,2>>(), stringliteral("-(1/2)")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<1,2>>(), stringliteral("(1/2)")) == 0);
            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<3,2>>(), stringliteral("(3/2)")) == 0);

            //log && log(xtag("ratio<2>", stringliteral_from_ratio<std::ratio<2>>().c_str()));
            static_assert(stringliteral_compare(stringliteral_from_ratio<std::ratio<2>>(), stringliteral("2")) == 0);

            static_assert(stringliteral_compare(bpu_assemble_abbrev_helper<dim::mass, std::ratio<1>, std::ratio<1>>(), stringliteral("g")) == 0);
            //log && log(xtag("s^(-1/2)", bpu_assemble_abbrev_helper<dim::time, std::ratio<1>, std::ratio<-1,2>>().c_str()));
            static_assert(stringliteral_compare(bpu_assemble_abbrev_helper<dim::time, std::ratio<1>, std::ratio<-1,2>>(), stringliteral("s^-(1/2)")) == 0);
            //stringliteral_compare(stringliteral_from_ratio<std::ratio<2>>(), stringliteral("^2")) == 0);
#endif

            //static_assert(stringliteral_compare(stringliteral_from_int_v<10>(), obs::stringliteral("10")) == 0);

            //REQUIRE(strcmp(obs::stringliteral_from_digit(1).value_, "1") == 0);
            //REQUIRE(strcmp(obs::ratio2str<std::ratio<1>>().value_, "") == 0);

        } /*TEST_CASE(native_dim_abbrev)*/

        TEST_CASE("dimension", "[dimension]") {
            constexpr bool c_debug_flag = true;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.dimension"));
            //log && log("(A)", xtag("foo", foo));

            using t1 = unit::bpu<unit::dim::currency, std::ratio<1,1>>;

            static_assert(t1::c_native_dim == unit::dim::currency);
            static_assert(t1::power_type::num == 1);
            static_assert(t1::power_type::den == 1);

            using t2 = unit::bpu<unit::dim::time, std::ratio<1>, std::ratio<-1,2>>;

            static_assert(t2::c_native_dim == unit::dim::time);
            static_assert(t2::power_type::num == -1);
            static_assert(t2::power_type::den == 2);

            using dim1 = wrap_unit<std::ratio<1>, bpu_node<t1>>;
            using d1 = dim1::dim_type; /* ccy */
            REQUIRE(unused_same<d1::front_type, t1>());
            REQUIRE(unused_same<unit::lookup_bpu<d1, 0>::power_unit_type, t1>());
#ifdef NOT_USING
            static_assert(unit::lo_basis_elt_of<d1>::c_lo_basis == t1::c_basis);
#endif

            static_assert(unit::native_lo_bwp_of<d1>::bwp_type::c_index == 0);
            static_assert(unit::native_lo_bwp_of<d1>::bwp_type::c_basis == unit::dim::currency);


            using dim2 = wrap_unit<std::ratio<1>, bpu_node<t2>>;
            using d2 = dim2::dim_type; /* t^(-1/2) */
            REQUIRE(unused_same<d2::front_type, t2>());
            REQUIRE(unused_same<unit::lookup_bpu<d2, 0>::power_unit_type, t2>());
            static_assert(unit::native_lo_bwp_of<d2>::bwp_type::c_index == 0);
            static_assert(unit::native_lo_bwp_of<d2>::bwp_type::c_basis == unit::dim::time);

            using dim3 = wrap_unit<std::ratio<1>, bpu_node<t1, bpu_node<t2>>>;
            using d3 = dim3::dim_type; /* ccy.t^(-1/2) */
            REQUIRE(unused_same<unit::lookup_bpu<d3, 0>::power_unit_type, t1>());

            {
                using type = unit::lookup_bpu<d3, 1>::power_unit_type;
                //std::cerr << "unit::power_unit_of<d3,1>::power_unit_type" << xtag("type",  reflect::type_name<type>()) << std::endl;

                REQUIRE(unused_same<type, t2>());
            }

#ifdef NOT_USING
            static_assert(unit::lo_basis_elt_of<d3>::c_lo_basis == t2::c_basis);
#endif

            /* lowest is in pos 1,  beacuse t2=time before t1=currency */
            static_assert(unit::native_lo_bwp_of<d3>::bwp_type::c_index == 1);

            static_assert(unused_same<unit::without_elt<d3, 0>::dim_type, d2>());
            //using type = unit::without_elt<d3, 1>::dim_type;
            //std::cerr << "unit::without_elt<d3,1>::dim_type" << xtag("type", reflect::type_name<type>()) << std::endl;
            static_assert(unused_same<unit::without_elt<d3, 1>::dim_type, d1>());


            using d3b = wrap_unit<std::ratio<1>,
                                  bpu_node<t2, bpu_node<t1>>>::dim_type; /* t^(-1/2).ccy */
            //using d3b = unit::dimension_impl<t2, unit::dimension_impl<t1>>; /* t^(-1/2).ccy */
            REQUIRE(unused_same<unit::lookup_bpu<d3b, 0>::power_unit_type, t2>());
            REQUIRE(unused_same<unit::lookup_bpu<d3b, 1>::power_unit_type, t1>());

            /* lowest is in pos 0 */
            static_assert(unit::native_lo_bwp_of<d3b>::bwp_type::c_index == 0);

            static_assert(unused_same<unit::without_elt<d3b, 0>::dim_type, d1>());
            static_assert(unused_same<unit::without_elt<d3b, 1>::dim_type, d2>());

            static_assert(unused_same<unit::canonical_t<d3>, unit::canonical_t<d3b>>());

            log && log(xtag("d1.abbrev", unit_abbrev_v<dim1>.c_str()));
            log && log(xtag("d2.abbrev", unit_abbrev_v<dim2>.c_str()));
            log && log(xtag("d3.abbrev", unit_abbrev_v<dim3>.c_str()));
        }

        TEST_CASE("dimension2", "[dimension2]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.dimension2"));
            //log && log("(A)", xtag("foo", foo));

            using di = di_cartesian_product<typename gram::dim_type, typename second::dim_type>;

            log && log(xtag("di", Reflect::require<di>()->canonical_name()));
            log && log(xtag("di::outer_scalefactor_type", Reflect::require<di::outer_scalefactor_type>()->canonical_name()));
            log && log(xtag("di::bpu_list_type", Reflect::require<di::bpu_list_type>()->canonical_name()));

            using u1 = unit_cartesian_product_t<gram, second>;

            log && log(xtag("u1", Reflect::require<u1>()->canonical_name()));

            log && log(xtag("u1", ccs(unit_abbrev_v<u1>.value_)));
        } /*TEST_CASE(dimension2)*/

        TEST_CASE("dimension3", "[dimension3]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.dimension3"));
            //log && log("(A)", xtag("foo", foo));

            using u1 = unit_invert_t<second>;

            log && log(xtag("second^-1", Reflect::require<u1>()->canonical_name()));
            log && log(xtag("u1", unit_abbrev_v<u1>.c_str()));

            REQUIRE(strcmp(unit_abbrev_v<u1>.c_str(), "s^-1") == 0);

            using u2 = second;

            log && log(xtag("second", Reflect::require<u2>()->canonical_name()));
            log && log(xtag("u2", unit_abbrev_v<u2>.c_str()));

            using u1u2 = unit_cartesian_product_t<u1, u2>;

            log && log(xtag("u1u2", Reflect::require<u1u2>()->canonical_name()));

#ifdef NOT_USING
            using di1 = d1::dim_type;
            using di2 = d2::dim_type;
            using di1di2 = di_cartesian_product<di1,di2>::type;

            log && log(xtag("di1di2", Reflect::require<di1di2>()->canonical_name()));
#endif

            using f1 = u1::dim_type::front_type;
            using r1 = u1::dim_type::rest_type;
            using tmp = di_cartesian_product1<f1, r1, u2::dim_type>;

            log && log(xtag("f1", Reflect::require<f1>()->canonical_name()));
            log && log(xtag("r1", Reflect::require<r1>()->canonical_name()));
            log && log(xtag("(f1.r1).outer_scalefactor_type", Reflect::require<tmp::outer_scalefactor_type>()->canonical_name()));
            log && log(xtag("(f1.r1).bpu_list_type", Reflect::require<tmp::bpu_list_type>()->canonical_name()));

            using tmp2 = bpu_cartesian_product<f1, u2::dim_type>;

            log && log(xtag("(f1.u2).outer_scalefactor_type", Reflect::require<tmp2::outer_scalefactor_type>()->canonical_name()));
            log && log(xtag("(f1.u2).bpu_list_type", Reflect::require<tmp2::bpu_list_type>()->canonical_name()));

            using f2 = u2::dim_type::front_type;
            log && log(xtag("f2", Reflect::require<f2>()->canonical_name()));

            using tmp3 = bpu_cartesian_product_helper<f1, f2, void>;
            log && log(xtag("(f1.f2).outer_scalefactor_type", Reflect::require<tmp3::outer_scalefactor_type>()->canonical_name()));
            log && log(xtag("(f1.f2).bpu_list_type", Reflect::require<tmp3::bpu_list_type>()->canonical_name()));
        } /*TEST_CASE(dimension3)*/


    } /*namespace ut*/

} /*namespace xo*/


/* end dimension.test.cpp */
