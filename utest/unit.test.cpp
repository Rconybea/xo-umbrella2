/* @file dimension.test.cpp */

#include "xo/unit/unit.hpp"
#include "xo/unit/Quantity2_iostream.hpp"
#include "xo/unit/Quantity2.hpp"
#include "xo/unit/scaled_unit_iostream.hpp"
#include "xo/unit/natural_unit.hpp"
#include "xo/unit/natural_unit_iostream.hpp"
#include "xo/unit/bpu_store.hpp"
#include "xo/unit/native_bpu2.hpp"
#include "xo/unit/native_bpu2_iostream.hpp"
#include "xo/unit/basis_unit2.hpp"
#include "xo/unit/dim_util2.hpp"
#include "xo/reflect/Reflect.hpp"
//#include "xo/cxxutil/demangle.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace ut {
        /* compile-time tests */

        using xo::reflect::Reflect;

        namespace su2 = xo::unit::su2;

        using xo::unit::Quantity2;
        using xo::unit::dim;
        using xo::unit::basis_unit2_abbrev_type;
        using xo::unit::native_unit2;
        using xo::unit::native_unit2_v;
        using xo::unit::scalefactor_ratio_type;
        using xo::unit::units::scaled_native_unit2_abbrev;
        using xo::unit::units::scaled_native_unit2_abbrev_v;
        using xo::unit::basis_unit2;
        using xo::unit::abbrev::basis_unit2_abbrev;;
        using xo::unit::bpu2_abbrev_type;
        using xo::unit::abbrev::bpu2_abbrev;
        using xo::unit::basis_unit2_store;
        using xo::unit::power_ratio_type;
        using xo::unit::abbrev::flatstring_from_exponent;
        using xo::unit::bpu2;
        using xo::unit::detail::bpu2_rescale;
        using xo::unit::detail::bpu2_product;
        using xo::unit::natural_unit;
        using xo::unit::bpu_array_maker;
        using xo::unit::detail::nu_product;
        using xo::unit::unit_qty;

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

        TEST_CASE("basis_unit2", "[basis_unit2]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.basis_unit2"));
            //log && log("(A)", xtag("foo", foo));

            static_assert(native_unit2_v[static_cast<int>(dim::mass)].native_dim() == dim::mass);
            static_assert(native_unit2_v[static_cast<int>(dim::distance)].native_dim() == dim::distance);
            static_assert(native_unit2_v[static_cast<int>(dim::time)].native_dim() == dim::time);
            static_assert(native_unit2_v[static_cast<int>(dim::time)].native_dim() == dim::time);
            static_assert(native_unit2_v[static_cast<int>(dim::currency)].native_dim() == dim::currency);
            static_assert(native_unit2_v[static_cast<int>(dim::price)].native_dim() == dim::price);

            log && log(xtag("mass*10^3", basis_unit2_abbrev(dim::mass, scalefactor_ratio_type(1000, 1))));

            static_assert(basis_unit2_abbrev(dim::mass, scalefactor_ratio_type(1000, 1))
                          == basis_unit2_abbrev_type::from_chars("kg"));

            log && log("---------------------");

            static_assert(basis_unit2(dim::mass, scalefactor_ratio_type(1, 1000000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("ng"));
            static_assert(basis_unit2(dim::mass, scalefactor_ratio_type(1, 1000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("ug"));
            static_assert(basis_unit2(dim::mass, scalefactor_ratio_type(1, 1000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("mg"));
            static_assert(basis_unit2(dim::mass, scalefactor_ratio_type(1, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("g"));
            static_assert(basis_unit2(dim::mass, scalefactor_ratio_type(1000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("kg"));
            static_assert(basis_unit2(dim::mass, scalefactor_ratio_type(1000000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("t"));
            static_assert(basis_unit2(dim::mass, scalefactor_ratio_type(1000000000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("kt"));

            log && log(xtag("distance", basis_unit2_abbrev(dim::distance, scalefactor_ratio_type(1, 1))));

            static_assert(basis_unit2(dim::distance, scalefactor_ratio_type(1, 1000000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("nm"));
            static_assert(basis_unit2(dim::distance, scalefactor_ratio_type(1, 1000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("um"));
            static_assert(basis_unit2(dim::distance, scalefactor_ratio_type(1, 1000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("mm"));
            static_assert(basis_unit2(dim::distance, scalefactor_ratio_type(1, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("m"));
            static_assert(basis_unit2(dim::distance, scalefactor_ratio_type(1000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("km"));
            static_assert(basis_unit2(dim::distance, scalefactor_ratio_type(1000000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("Mm"));
            static_assert(basis_unit2(dim::distance, scalefactor_ratio_type(1000000000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("Gm"));

            log && log(xtag("time", basis_unit2_abbrev(dim::time, scalefactor_ratio_type(1, 1))));

            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(1, 1000000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("ns"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(1, 1000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("us"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(1, 1000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("us"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(1, 1000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("ms"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(1, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("s"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(60, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("min"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("hr"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("dy"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(7*24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("wk"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(30*24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("mo"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(250*24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("yr250"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(360*24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("yr360"));
            static_assert(basis_unit2(dim::time, scalefactor_ratio_type(365*24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("yr365"));

            log && log("---------------------");

            log && log(xtag("mass*10^-9",       scaled_native_unit2_abbrev_v<dim::mass,               1, 1000000000>));
            log && log(xtag("mass*10^-6",       scaled_native_unit2_abbrev_v<dim::mass,               1,    1000000>));
            log && log(xtag("mass*10^-3",       scaled_native_unit2_abbrev_v<dim::mass,               1,       1000>));
            log && log(xtag("mass",             scaled_native_unit2_abbrev_v<dim::mass>));
            log && log(xtag("mass*10^3",        scaled_native_unit2_abbrev_v<dim::mass,            1000,          1>));
            log && log(xtag("mass*10^6",        scaled_native_unit2_abbrev_v<dim::mass,         1000000,          1>));
            log && log(xtag("mass*10^9",        scaled_native_unit2_abbrev_v<dim::mass,      1000000000,          1>));

            log && log(xtag("distance*10^-9",   scaled_native_unit2_abbrev_v<dim::distance,           1, 1000000000>));
            log && log(xtag("distance*10^-6",   scaled_native_unit2_abbrev_v<dim::distance,           1,    1000000>));
            log && log(xtag("distance*10^-3",   scaled_native_unit2_abbrev_v<dim::distance,           1,       1000>));
            log && log(xtag("distance",         scaled_native_unit2_abbrev_v<dim::distance>));
            log && log(xtag("distance*10^3",    scaled_native_unit2_abbrev_v<dim::distance,        1000,          1>));

            log && log(xtag("time*10^-9",       scaled_native_unit2_abbrev_v<dim::time,               1, 1000000000>));
            log && log(xtag("time*10^-6",       scaled_native_unit2_abbrev_v<dim::time,               1,    1000000>));
            log && log(xtag("time*10^-3",       scaled_native_unit2_abbrev_v<dim::time,               1,       1000>));
            log && log(xtag("time",             scaled_native_unit2_abbrev_v<dim::time>));
            log && log(xtag("time*60",          scaled_native_unit2_abbrev_v<dim::time,              60,          1>));
            log && log(xtag("time*3600",        scaled_native_unit2_abbrev_v<dim::time,            3600,          1>));
            log && log(xtag("time*24*3600",     scaled_native_unit2_abbrev_v<dim::time,         24*3600,          1>));
            log && log(xtag("time*250*24*3600", scaled_native_unit2_abbrev_v<dim::time,     250*24*3600,          1>));
            log && log(xtag("time*360*24*3600", scaled_native_unit2_abbrev_v<dim::time,     360*24*3600,          1>));
            log && log(xtag("time*365*24*3600", scaled_native_unit2_abbrev_v<dim::time,     365*24*3600,          1>));

            log && log(xtag("currency",         scaled_native_unit2_abbrev_v<dim::currency>));

            log && log(xtag("price",            scaled_native_unit2_abbrev_v<dim::price>));

            REQUIRE(xo::unit::units::scaled_native_unit2_abbrev<dim::mass>::value == xo::flatstring("g"));

            /* proof that scaled_native_unit2_abbrev::value is constexpr */
            static_assert(scaled_native_unit2_abbrev_v<dim::mass>
                          == basis_unit2_abbrev_type::from_flatstring(xo::flatstring("g")));

        } /*TEST_CASE(basis_unit2)*/

        TEST_CASE("basis_unit2_store", "[basis_unit2_store]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.basis_unit2_store"));
            //log && log("(A)", xtag("foo", foo));

            basis_unit2_store<class AnyTag> bu_store;

            log && log(xtag("mass*10^-9",       bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(          1, 1000000000))));
            log && log(xtag("mass*10^-6",       bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(          1,    1000000))));
            log && log(xtag("mass*10^-3",       bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(          1,       1000))));
            log && log(xtag("mass",             bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(          1,          1))));
            log && log(xtag("mass*10^3",        bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(       1000,          1))));
            log && log(xtag("mass*10^6",        bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(    1000000,          1))));
            log && log(xtag("mass*10^9",        bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type( 1000000000,          1))));

            log && log(xtag("distance*10^-9",   bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(          1, 1000000000))));
            log && log(xtag("distance*10^-6",   bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(          1,    1000000))));
            log && log(xtag("distance*10^-3",   bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(          1,       1000))));
            log && log(xtag("distance",         bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(          1,          1))));
            log && log(xtag("distance*10^3",    bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(       1000,          1))));

            log && log(xtag("time*10^-9",       bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(          1, 1000000000))));
            log && log(xtag("time*10^-6",       bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(          1,    1000000))));
            log && log(xtag("time*10^-3",       bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(          1,       1000))));
            log && log(xtag("time",             bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(          1,          1))));
            log && log(xtag("time*60",          bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(         60,          1))));
            log && log(xtag("time*3600",        bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(       3600,          1))));
            log && log(xtag("time*24*3600",     bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(    24*3600,          1))));
            log && log(xtag("time*250*24*3600", bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(250*24*3600,          1))));
            log && log(xtag("time*360*24*3600", bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(360*24*3600,          1))));
            log && log(xtag("time*365*24*3600", bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(365*24*3600,          1))));

            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(          1, 1000000000)).c_str(), "ng") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(          1,    1000000)).c_str(), "ug") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(          1,       1000)).c_str(), "mg") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(          1,          1)).c_str(), "g") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(       1000,          1)).c_str(), "kg") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type(    1000000,          1)).c_str(), "t") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::mass,     scalefactor_ratio_type( 1000000000,          1)).c_str(), "kt") == 0);

            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(          1, 1000000000)).c_str(), "nm") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(          1,    1000000)).c_str(), "um") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(          1,       1000)).c_str(), "mm") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(          1,          1)).c_str(), "m") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::distance, scalefactor_ratio_type(       1000,          1)).c_str(), "km") == 0);

            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(          1, 1000000000)).c_str(), "ns") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(          1,    1000000)).c_str(), "us") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(          1,       1000)).c_str(), "ms") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(          1,          1)).c_str(), "s") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(         60,          1)).c_str(), "min") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(       3600,          1)).c_str(), "hr") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(    24*3600,          1)).c_str(), "dy") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(250*24*3600,          1)).c_str(), "yr250") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(360*24*3600,          1)).c_str(), "yr360") == 0);
            REQUIRE(::strcmp(bu_store.bu_abbrev(dim::time,     scalefactor_ratio_type(365*24*3600,          1)).c_str(), "yr365") == 0);

        } /*TEST_CASE(basis_unit2_store)*/

        TEST_CASE("flatstring_from_exponent", "[flatstring_from_exponent]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.flatstring_from_exponent"));
            //log && log("(A)", xtag("foo", foo));

            log && log(xtag("^-3", flatstring_from_exponent(-3,1)));
            log && log(xtag("^-2", flatstring_from_exponent(-2,1)));
            log && log(xtag("^-1", flatstring_from_exponent(-1,1)));
            log && log(xtag("^-1/2", flatstring_from_exponent(-1,2)));
            log && log(xtag("^0", flatstring_from_exponent(0,1)));
            log && log(xtag("^1/2", flatstring_from_exponent(1,2)));
            log && log(xtag("^1", flatstring_from_exponent(1,1)));
            log && log(xtag("^2", flatstring_from_exponent(2,1)));
            log && log(xtag("^3", flatstring_from_exponent(3,1)));

            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(-3,1))
                          == flatstring<5>::from_flatstring(flatstring("^-3")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(-2,1))
                          == flatstring<5>::from_flatstring(flatstring("^-2")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(-1,1))
                          == flatstring<5>::from_flatstring(flatstring("^-1")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(-1,2))
                          == flatstring<5>::from_flatstring(flatstring("^(-1/2)")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(1,2))
                          == flatstring<5>::from_flatstring(flatstring("^(1/2)")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(1,1))
                          == flatstring<5>::from_flatstring(flatstring("")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(2,1))
                          == flatstring<5>::from_flatstring(flatstring("^2")));
            static_assert(flatstring<5>::from_flatstring(flatstring_from_exponent(3,1))
                          == flatstring<5>::from_flatstring(flatstring("^3")));
        } /*TEST_CASE(flatstring_from_exponent)*/

        TEST_CASE("bpu2_abbrev", "[bpu2_abbrev]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu2_assemble_abbrev"));
            //log && log("(A)", xtag("foo", foo));

            log && log(xtag("1/(kg*kg)", bpu2_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-2, 1))));
            log && log(xtag("1/kg", bpu2_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-1, 1))));
            log && log(xtag("kg", bpu2_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1))));
            log && log(xtag("kg*kg", bpu2_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(2, 1))));

            static_assert(bpu2<int64_t>(dim::mass, scalefactor_ratio_type(1, 1), power_ratio_type(1, 1)).abbrev()
                          == bpu2_abbrev_type::from_chars("g"));
            static_assert(bpu2<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1)).abbrev()
                          == bpu2_abbrev_type::from_chars("kg"));
            static_assert(bpu2<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu2_abbrev_type::from_chars("kg^-1"));
            static_assert(bpu2<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-2, 1)).abbrev()
                          == bpu2_abbrev_type::from_chars("kg^-2"));

            static_assert(bpu2<int64_t>(dim::time, scalefactor_ratio_type(60, 1), power_ratio_type(-2, 1)).abbrev()
                          == bpu2_abbrev_type::from_chars("min^-2"));
            static_assert(bpu2<int64_t>(dim::time, scalefactor_ratio_type(3600, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu2_abbrev_type::from_chars("hr^-1"));
            static_assert(bpu2<int64_t>(dim::time, scalefactor_ratio_type(24*3600, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu2_abbrev_type::from_chars("dy^-1"));
            static_assert(bpu2<int64_t>(dim::time, scalefactor_ratio_type(360*24*3600, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu2_abbrev_type::from_chars("yr360^-1"));
            static_assert(bpu2<int64_t>(dim::time, scalefactor_ratio_type(360*24*3600, 1), power_ratio_type(-1, 2)).abbrev()
                          == bpu2_abbrev_type::from_chars("yr360^(-1/2)"));
        } /*TEST_CASE(bpu2_abbrev)*/


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

        TEST_CASE("bpu_rescale", "[bpu_rescale]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_rescale"));
            //log && log("(A)", xtag("foo", foo));

            /* keep spelled-out test.  Will generalize to fractional powers when c++26 available */
            {
                constexpr auto p = power_ratio_type(1, 1);

                constexpr auto orig_bpu = bpu2<int64_t>(dim::mass,
                                                        scalefactor_ratio_type(1000, 1),
                                                        power_ratio_type(1, 1));
                static_assert(orig_bpu.native_dim() == dim::mass);

                constexpr auto new_scalefactor = scalefactor_ratio_type(1000000, 1);

                constexpr auto mult = orig_bpu.scalefactor() / new_scalefactor;
                static_assert(mult.num() == 1);
                static_assert(mult.den() == 1000);

                constexpr auto p_floor = orig_bpu.power().floor();
                static_assert(p_floor == 1);

                constexpr auto p_frac = orig_bpu.power().frac().template to<double>();
                static_assert(p_frac == 0.0);

                constexpr auto outer_sf_exact = mult.power(p_floor);
                static_assert(outer_sf_exact.num() == 1);
                static_assert(outer_sf_exact.den() == 1000);

                constexpr auto mult_inexact = mult.template to<double>();
                static_assert(mult_inexact == 0.001);

                constexpr auto rr = bpu2_rescale<int64_t>(orig_bpu, scalefactor_ratio_type(1000000, 1));

                static_assert(rr.bpu_rescaled_.power() == power_ratio_type(1,1));
                static_assert(rr.outer_scale_exact_ == outer_sf_exact);
                static_assert(rr.outer_scale_sq_ == 1.0);
            }

            /* keep spelled-out test.  Will generalize to other fractional powers when c++26 available */
            {
                constexpr auto p = power_ratio_type(-1, 2);

                constexpr auto orig_bpu = bpu2<int64_t>(dim::time,
                                                        scalefactor_ratio_type(360*24*3600, 1),
                                                        power_ratio_type(-1, 2));
                static_assert(orig_bpu.native_dim() == dim::time);

                constexpr auto new_scalefactor = scalefactor_ratio_type(30*24*3600, 1);

                constexpr auto mult = orig_bpu.scalefactor() / new_scalefactor;
                log && log(xtag("mult", mult));
                static_assert(mult.num() == 12);
                static_assert(mult.den() == 1);

                constexpr auto p_floor = orig_bpu.power().floor();
                static_assert(p_floor == 0);

                constexpr auto p_frac = orig_bpu.power().frac().template to<double>();
                static_assert(p_frac == -0.5);

                constexpr auto outer_sf_exact = mult.power(p_floor);
                static_assert(outer_sf_exact.num() == 1);
                static_assert(outer_sf_exact.den() == 1);

                constexpr auto mult_inexact = mult.template to<double>();
                static_assert(mult_inexact == 12.0);

                constexpr auto rr = bpu2_rescale<int64_t>(orig_bpu, scalefactor_ratio_type(30*24*3600, 1));

                log && log(xtag("rr.outer_scale_exact", rr.outer_scale_exact_),
                           xtag("rr.outer_scale_sq", rr.outer_scale_sq_));

                static_assert(rr.bpu_rescaled_.power() == power_ratio_type(-1,2));
                static_assert(rr.outer_scale_exact_ == outer_sf_exact);
                static_assert(rr.outer_scale_sq_ == 12.0);
            }

            /* keep spelled-out test.  Will generalize to other fractional powers when c++26 available */
            {
                constexpr auto p = power_ratio_type(-3, 2);

                constexpr auto orig_bpu = bpu2<int64_t>(dim::time,
                                                        scalefactor_ratio_type(360*24*3600, 1),
                                                        power_ratio_type(-3, 2));
                static_assert(orig_bpu.native_dim() == dim::time);

                constexpr auto new_scalefactor = scalefactor_ratio_type(30*24*3600, 1);

                constexpr auto mult = orig_bpu.scalefactor() / new_scalefactor;
                log && log(xtag("mult", mult));
                static_assert(mult.num() == 12);
                static_assert(mult.den() == 1);

                constexpr auto p_floor = orig_bpu.power().floor();
                static_assert(p_floor == -1);

                constexpr auto p_frac = orig_bpu.power().frac().template to<double>();
                static_assert(p_frac == -0.5);

                constexpr auto outer_sf_exact = mult.power(p_floor);
                static_assert(outer_sf_exact.num() == 1);
                static_assert(outer_sf_exact.den() == 12);

                constexpr auto mult_inexact = mult.template to<double>();
                static_assert(mult_inexact == 12.0);

                constexpr auto rr = bpu2_rescale<int64_t>(orig_bpu, scalefactor_ratio_type(30*24*3600, 1));

                log && log(xtag("rr.outer_scale_exact", rr.outer_scale_exact_),
                           xtag("rr.outer_scale_sq", rr.outer_scale_sq_));

                static_assert(rr.bpu_rescaled_.power() == power_ratio_type(-3,2));
                static_assert(rr.outer_scale_exact_ == outer_sf_exact);
                static_assert(rr.outer_scale_sq_ == 12.0);
            }
        } /*TEST_CASE(bpu_rescale)*/

        TEST_CASE("bpu_product", "[bpu_product]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_product"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr auto bpu_x = bpu2<int64_t>(dim::time,
                                                     scalefactor_ratio_type(360*24*3600, 1),
                                                     power_ratio_type(-3,2));
                static_assert(bpu_x.native_dim() == dim::time);

                constexpr auto bpu_y = bpu2<int64_t>(dim::time,
                                                     scalefactor_ratio_type(360*24*3600, 1),
                                                     power_ratio_type(1,2));
                static_assert(bpu_y.native_dim() == dim::time);

                constexpr auto bpu_prod = bpu2_product<int64_t>(bpu_x, bpu_y);

                log && log(xtag("bpu_prod.bpu_rescaled", bpu_prod.bpu_rescaled_));
                log && log(xtag("bpu_prod.outer_scale_exact", bpu_prod.outer_scale_exact_));
                log && log(xtag("bpu_prod.outer_scale_sq", bpu_prod.outer_scale_sq_));

                static_assert(bpu_prod.bpu_rescaled_.native_dim() == dim::time);
                static_assert(bpu_prod.bpu_rescaled_.scalefactor() == scalefactor_ratio_type(360*24*3600, 1));
                static_assert(bpu_prod.bpu_rescaled_.power() == power_ratio_type(-1, 1));
                static_assert(bpu_prod.outer_scale_exact_ == scalefactor_ratio_type(1,1));
                static_assert(bpu_prod.outer_scale_sq_ == 1.0);
            }
        } /*TEST_CASE(bpu_product)*/

        TEST_CASE("bpu_product2", "[bpu_product]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_product2"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr auto bpu_x = bpu2<int64_t>(dim::time,
                                                     scalefactor_ratio_type(360*24*3600, 1),
                                                     power_ratio_type(-3,2));
                static_assert(bpu_x.native_dim() == dim::time);

                constexpr auto bpu_y = bpu2<int64_t>(dim::time,
                                                     scalefactor_ratio_type(30*24*3600, 1),
                                                     power_ratio_type(1,2));
                static_assert(bpu_y.native_dim() == dim::time);

                constexpr auto bpu_prod = bpu2_product<int64_t>(bpu_x, bpu_y);

                log && log(xtag("bpu_prod.bpu_rescaled", bpu_prod.bpu_rescaled_));
                log && log(xtag("bpu_prod.outer_scale_exact", bpu_prod.outer_scale_exact_));
                log && log(xtag("bpu_prod.outer_scale_sq", bpu_prod.outer_scale_sq_));

                static_assert(bpu_prod.bpu_rescaled_.native_dim() == dim::time);
                static_assert(bpu_prod.bpu_rescaled_.scalefactor() == scalefactor_ratio_type(360*24*3600, 1));
                static_assert(bpu_prod.bpu_rescaled_.power() == power_ratio_type(-1, 1));
                static_assert(bpu_prod.outer_scale_exact_ == scalefactor_ratio_type(1,1));
                static_assert(bpu_prod.outer_scale_sq_ == 1.0/12.0);
            }
        } /*TEST_CASE(bpu_product2)*/

        TEST_CASE("bpu_array", "[bpu_array]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_array"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v;

                static_assert(v.n_bpu() == 0);
            }

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu2<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1))));

                static_assert(v.n_bpu() == 1);
            }

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu2<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu2<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);
            }
        } /*TEST_CASE(bpu_array)*/

        TEST_CASE("bpu_array_product0", "[bpu_array_product]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_array_product0"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu2<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu2<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);

                constexpr bpu2<int64_t> bpu(dim::time,
                                            scalefactor_ratio_type(250*24*3600, 1),
                                            power_ratio_type(-1, 2));

                static_assert(bpu.power() == power_ratio_type(-1, 2));

                constexpr auto prod_rr = nu_product(v, bpu);

                log && log(xtag("prod_rr.bpu_array", prod_rr.natural_unit_));
                log && log(xtag("prod_rr.outer_scale_exact", prod_rr.outer_scale_exact_));
                log && log(xtag("prod_rr.outer_scale_sq", prod_rr.outer_scale_sq_));

                static_assert(prod_rr.natural_unit_.n_bpu() == 3);
                static_assert(prod_rr.natural_unit_[0].native_dim() == dim::distance);
                static_assert(prod_rr.natural_unit_[0].scalefactor() == scalefactor_ratio_type(1, 1000));
                static_assert(prod_rr.natural_unit_[0].power() == power_ratio_type(2, 1));
                static_assert(prod_rr.natural_unit_[1].native_dim() == dim::mass);
                static_assert(prod_rr.natural_unit_[1].scalefactor() == scalefactor_ratio_type(1, 1000));
                static_assert(prod_rr.natural_unit_[1].power() == power_ratio_type(-1, 1));
                static_assert(prod_rr.natural_unit_[2].native_dim() == dim::time);
                static_assert(prod_rr.natural_unit_[2].scalefactor() == scalefactor_ratio_type(250*24*3600, 1));
                static_assert(prod_rr.natural_unit_[2].power() == power_ratio_type(-1, 2));
                static_assert(prod_rr.outer_scale_exact_ == scalefactor_ratio_type(1, 1));
                static_assert(prod_rr.outer_scale_sq_ == 1.0);
            }
        } /*TEST_CASE(bpu_array_product0)*/

        TEST_CASE("bpu_array_product1", "[bpu_array_product]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_array_product1"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu2<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu2<int64_t>(dim::time, scalefactor_ratio_type(30*24*3600, 1), power_ratio_type(-1, 2))));

                static_assert(v.n_bpu() == 2);

                constexpr bpu2<int64_t> bpu(dim::time,
                                            scalefactor_ratio_type(360*24*3600, 1),
                                            power_ratio_type(-1, 2));

                static_assert(bpu.power() == power_ratio_type(-1, 2));

                constexpr auto prod_rr = nu_product(v, bpu);

                log && log(xtag("prod_rr.bpu_array", prod_rr.natural_unit_));
                log && log(xtag("prod_rr.outer_scale_exact", prod_rr.outer_scale_exact_));
                log && log(xtag("prod_rr.outer_scale_sq", prod_rr.outer_scale_sq_));

                static_assert(prod_rr.natural_unit_.n_bpu() == 2);
                static_assert(prod_rr.natural_unit_[0].native_dim() == dim::distance);
                static_assert(prod_rr.natural_unit_[0].scalefactor() == scalefactor_ratio_type(1, 1000));
                static_assert(prod_rr.natural_unit_[0].power() == power_ratio_type(2, 1));
                static_assert(prod_rr.natural_unit_[1].native_dim() == dim::time);
                static_assert(prod_rr.natural_unit_[1].scalefactor() == scalefactor_ratio_type(30*24*3600, 1));
                static_assert(prod_rr.natural_unit_[1].power() == power_ratio_type(-1, 1));
                static_assert(prod_rr.outer_scale_exact_ == scalefactor_ratio_type(1, 1));
                static_assert(prod_rr.outer_scale_sq_ == 12.0);
            }
        } /*TEST_CASE(bpu_array_product1)*/

        TEST_CASE("bpu_array_product2", "[bpu_array_product]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_array_product2"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu2<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu2<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);

                constexpr bpu2<int64_t> bpu(dim::distance,
                                            scalefactor_ratio_type(1, 1000),
                                            power_ratio_type(-1, 1));

                static_assert(bpu.power() == power_ratio_type(-1, 1));

                constexpr auto prod_rr = nu_product(v, bpu);

                log && log(xtag("prod_rr.bpu_array", prod_rr.natural_unit_));
                log && log(xtag("prod_rr.outer_scale_exact", prod_rr.outer_scale_exact_));
                log && log(xtag("prod_rr.outer_scale_sq", prod_rr.outer_scale_sq_));

                static_assert(prod_rr.natural_unit_.n_bpu() == 2);
                static_assert(prod_rr.natural_unit_[0].native_dim() == dim::distance);
                static_assert(prod_rr.natural_unit_[0].scalefactor() == scalefactor_ratio_type(1, 1000));
                static_assert(prod_rr.natural_unit_[0].power() == power_ratio_type(1, 1));
                static_assert(prod_rr.natural_unit_[1].native_dim() == dim::mass);
                static_assert(prod_rr.natural_unit_[1].scalefactor() == scalefactor_ratio_type(1, 1000));
                static_assert(prod_rr.natural_unit_[1].power() == power_ratio_type(-1, 1));
                static_assert(prod_rr.outer_scale_exact_ == scalefactor_ratio_type(1, 1));
                static_assert(prod_rr.outer_scale_sq_ == 1.0);
            }
        } /*TEST_CASE(bpu_array_product2)*/

        TEST_CASE("bpu_array_product3", "[bpu_array_product]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu_array_product3"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu2<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu2<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);

                constexpr natural_unit<int64_t> w
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu2<int64_t>(dim::time, scalefactor_ratio_type(30*24*3600, 1), power_ratio_type(-1, 2))));

                static_assert(w.n_bpu() == 1);

                constexpr auto prod_rr = nu_product(v, w);

                log && log(xtag("prod_rr.bpu_array", prod_rr.natural_unit_));
                log && log(xtag("prod_rr.outer_scale_exact", prod_rr.outer_scale_exact_));
                log && log(xtag("prod_rr.outer_scale_sq", prod_rr.outer_scale_sq_));

                static_assert(prod_rr.natural_unit_.n_bpu() == 3);
                static_assert(prod_rr.natural_unit_[0].native_dim() == dim::distance);
                static_assert(prod_rr.natural_unit_[0].scalefactor() == scalefactor_ratio_type(1, 1000));
                static_assert(prod_rr.natural_unit_[0].power() == power_ratio_type(2, 1));
                static_assert(prod_rr.natural_unit_[1].native_dim() == dim::mass);
                static_assert(prod_rr.natural_unit_[1].scalefactor() == scalefactor_ratio_type(1, 1000));
                static_assert(prod_rr.natural_unit_[1].power() == power_ratio_type(-1, 1));
                static_assert(prod_rr.natural_unit_[2].native_dim() == dim::time);
                static_assert(prod_rr.natural_unit_[2].scalefactor() == scalefactor_ratio_type(30*24*3600, 1));
                static_assert(prod_rr.natural_unit_[2].power() == power_ratio_type(-1, 2));
                static_assert(prod_rr.outer_scale_exact_ == scalefactor_ratio_type(1, 1));
                static_assert(prod_rr.outer_scale_sq_ == 1.0);
            }
        } /*TEST_CASE(bpu_array_product3)*/

        TEST_CASE("scaled_unit0", "[scaled_unit0]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.scaled_unit0"));
            //log && log("(A)", xtag("foo", foo));

            constexpr auto ng = su2::nanogram;
            constexpr auto ng2 = ng * ng;

            log && log(xtag("ng", ng));
            log && log(xtag("ng*ng", ng2));
            //log && log(xtag("ng/ng",

            static_assert(ng.natural_unit_.n_bpu() == 1);
            static_assert(ng2.natural_unit_.n_bpu() == 1);
        } /*TEST_CASE(scaled_unit0)*/

        TEST_CASE("Quantity", "[Quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.Quantity"));
            //log && log("(A)", xtag("foo", foo));

            /* not constexpr until c++26 */
            auto ng = unit_qty(su2::nanogram);

            log && log(xtag("ng", ng));

            REQUIRE(ng.scale() == 1);
        } /*TEST_CASE(Quantity)*/

        TEST_CASE("Quantity2", "[Quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.Quantity2"));
            //log && log("(A)", xtag("foo", foo));

            /* not constexpr until c++26 */
            Quantity2 ng = unit_qty(su2::nanogram);
            auto ng2 = ng * ng;

            log && log(xtag("ng*ng", ng2));

            REQUIRE(ng2.scale() == 1);
        } /*TEST_CASE(Quantity2)*/

        TEST_CASE("Quantity3", "[Quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.Quantity3"));
            //log && log("(A)", xtag("foo", foo));

            /* not constexpr until c++26 */
            Quantity2 ng = unit_qty(su2::nanogram);
            Quantity2 ug = unit_qty(su2::microgram);

            {
                auto prod1 = ng * ug;
                log && log(xtag("ng*ug", prod1));

                /* units will be nanograms,  since that's on lhs */
                REQUIRE(prod1.unit().n_bpu() == 1);
                REQUIRE(prod1.unit()[0].native_dim() == dim::mass);
                REQUIRE(prod1.unit()[0].scalefactor() == scalefactor_ratio_type(1, 1000000000));
                REQUIRE(prod1.unit()[0].power() == power_ratio_type(2, 1));
                REQUIRE(prod1.scale() == 1000);
            }

            {
                auto prod2 = ug * ng;
                log && log(xtag("ug*ng", prod2));

                REQUIRE(prod2.unit().n_bpu() == 1);
                REQUIRE(prod2.unit()[0].native_dim() == dim::mass);
                REQUIRE(prod2.unit()[0].native_dim() == dim::mass);
                REQUIRE(prod2.unit()[0].scalefactor() == scalefactor_ratio_type(1, 1000000));
                REQUIRE(prod2.unit()[0].power() == power_ratio_type(2, 1));
                REQUIRE(prod2.scale() == 0.001);
            }

            //REQUIRE(ng2.scale() == 1);
        } /*TEST_CASE(Quantity3)*/

        TEST_CASE("dimension", "[dimension]") {
            constexpr bool c_debug_flag = false;

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
            constexpr bool c_debug_flag = false;

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
            constexpr bool c_debug_flag = false;

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
