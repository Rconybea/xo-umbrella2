/* @file unit.test.cpp */

#include "xo/unit/unit2.hpp"
#include "xo/unit/Quantity_iostream.hpp"
#include "xo/unit/Quantity.hpp"
#include "xo/unit/scaled_unit_iostream.hpp"
#include "xo/unit/natural_unit.hpp"
#include "xo/unit/natural_unit_iostream.hpp"
#include "xo/unit/bpu_store.hpp"
#include "xo/unit/bpu.hpp"
#include "xo/unit/bpu_iostream.hpp"
#include "xo/unit/basis_unit.hpp"
#include "xo/unit/native_unit.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace ut {
        /* compile-time tests */

        namespace su2 = xo::qty::su2;

        using xo::qty::Quantity;
        using xo::qty::dim;
        using xo::qty::basis_unit2_abbrev_type;
        using xo::qty::native_unit2_v;
        using xo::qty::scalefactor_ratio_type;
        using xo::qty::units::scaled_native_unit2_abbrev;
        using xo::qty::units::scaled_native_unit2_abbrev_v;
        using xo::qty::basis_unit;
        using xo::qty::abbrev::basis_unit2_abbrev;;
        using xo::qty::bpu_abbrev_type;
        using xo::qty::abbrev::bpu_abbrev;
        using xo::qty::basis_unit2_store;
        using xo::qty::power_ratio_type;
        using xo::qty::abbrev::flatstring_from_exponent;
        using xo::qty::bpu;
        using xo::qty::detail::bpu2_rescale;
        using xo::qty::detail::bpu2_product;
        using xo::qty::natural_unit;
        using xo::qty::bpu_array_maker;
        using xo::qty::detail::nu_product;
        using xo::qty::detail::nu_bpu_product;
        using xo::qty::detail::nu_ratio_inplace;
        using xo::qty::detail::nu_ratio;
        using xo::qty::unit_qty;

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

            static_assert(basis_unit(dim::mass, scalefactor_ratio_type(1, 1000000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("ng"));
            static_assert(basis_unit(dim::mass, scalefactor_ratio_type(1, 1000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("ug"));
            static_assert(basis_unit(dim::mass, scalefactor_ratio_type(1, 1000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("mg"));
            static_assert(basis_unit(dim::mass, scalefactor_ratio_type(1, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("g"));
            static_assert(basis_unit(dim::mass, scalefactor_ratio_type(1000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("kg"));
            static_assert(basis_unit(dim::mass, scalefactor_ratio_type(1000000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("t"));
            static_assert(basis_unit(dim::mass, scalefactor_ratio_type(1000000000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("kt"));

            log && log(xtag("distance", basis_unit2_abbrev(dim::distance, scalefactor_ratio_type(1, 1))));

            static_assert(basis_unit(dim::distance, scalefactor_ratio_type(1, 1000000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("nm"));
            static_assert(basis_unit(dim::distance, scalefactor_ratio_type(1, 1000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("um"));
            static_assert(basis_unit(dim::distance, scalefactor_ratio_type(1, 1000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("mm"));
            static_assert(basis_unit(dim::distance, scalefactor_ratio_type(1, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("m"));
            static_assert(basis_unit(dim::distance, scalefactor_ratio_type(1000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("km"));
            static_assert(basis_unit(dim::distance, scalefactor_ratio_type(1000000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("Mm"));
            static_assert(basis_unit(dim::distance, scalefactor_ratio_type(1000000000, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("Gm"));

            log && log(xtag("time", basis_unit2_abbrev(dim::time, scalefactor_ratio_type(1, 1))));

            static_assert(basis_unit(dim::time, scalefactor_ratio_type(1, 1000000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("ns"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(1, 1000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("us"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(1, 1000000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("us"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(1, 1000)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("ms"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(1, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("s"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(60, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("min"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("hr"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("dy"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(7*24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("wk"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(30*24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("mo"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(250*24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("yr250"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(360*24*3600, 1)).abbrev()
                          == basis_unit2_abbrev_type::from_chars("yr360"));
            static_assert(basis_unit(dim::time, scalefactor_ratio_type(365*24*3600, 1)).abbrev()
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

            REQUIRE(xo::qty::units::scaled_native_unit2_abbrev<dim::mass>::value == xo::flatstring("g"));

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

            log && log(xtag("1/(kg*kg)", bpu_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-2, 1))));
            log && log(xtag("1/kg", bpu_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-1, 1))));
            log && log(xtag("kg", bpu_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1))));
            log && log(xtag("kg*kg", bpu_abbrev(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(2, 1))));

            static_assert(bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1), power_ratio_type(1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("g"));
            static_assert(bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("kg"));
            static_assert(bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("kg^-1"));
            static_assert(bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(-2, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("kg^-2"));

            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(60, 1), power_ratio_type(-2, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("min^-2"));
            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(3600, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("hr^-1"));
            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(24*3600, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("dy^-1"));
            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(360*24*3600, 1), power_ratio_type(-1, 1)).abbrev()
                          == bpu_abbrev_type::from_chars("yr360^-1"));
            static_assert(bpu<int64_t>(dim::time, scalefactor_ratio_type(360*24*3600, 1), power_ratio_type(-1, 2)).abbrev()
                          == bpu_abbrev_type::from_chars("yr360^(-1/2)"));
        } /*TEST_CASE(bpu2_abbrev)*/


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

                constexpr auto orig_bpu = bpu<int64_t>(dim::mass,
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

                constexpr auto orig_bpu = bpu<int64_t>(dim::time,
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

                constexpr auto orig_bpu = bpu<int64_t>(dim::time,
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
                constexpr auto bpu_x = bpu<int64_t>(dim::time,
                                                     scalefactor_ratio_type(360*24*3600, 1),
                                                     power_ratio_type(-3,2));
                static_assert(bpu_x.native_dim() == dim::time);

                constexpr auto bpu_y = bpu<int64_t>(dim::time,
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
                constexpr auto bpu_x = bpu<int64_t>(dim::time,
                                                     scalefactor_ratio_type(360*24*3600, 1),
                                                     power_ratio_type(-3,2));
                static_assert(bpu_x.native_dim() == dim::time);

                constexpr auto bpu_y = bpu<int64_t>(dim::time,
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
                       (bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1))));

                static_assert(v.n_bpu() == 1);
            }

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

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
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);

                constexpr bpu<int64_t> bpu(dim::time,
                                            scalefactor_ratio_type(250*24*3600, 1),
                                            power_ratio_type(-1, 2));

                static_assert(bpu.power() == power_ratio_type(-1, 2));

                constexpr auto prod_rr = nu_bpu_product(v, bpu);

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
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu<int64_t>(dim::time, scalefactor_ratio_type(30*24*3600, 1), power_ratio_type(-1, 2))));

                static_assert(v.n_bpu() == 2);

                constexpr bpu<int64_t> bpu(dim::time,
                                            scalefactor_ratio_type(360*24*3600, 1),
                                            power_ratio_type(-1, 2));

                static_assert(bpu.power() == power_ratio_type(-1, 2));

                constexpr auto prod_rr = nu_bpu_product(v, bpu);

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
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);

                constexpr bpu<int64_t> bpu(dim::distance,
                                            scalefactor_ratio_type(1, 1000),
                                            power_ratio_type(-1, 1));

                static_assert(bpu.power() == power_ratio_type(-1, 1));

                constexpr auto prod_rr = nu_bpu_product(v, bpu);

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
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);

                constexpr natural_unit<int64_t> w
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu<int64_t>(dim::time, scalefactor_ratio_type(30*24*3600, 1), power_ratio_type(-1, 2))));

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

        TEST_CASE("natural_unit0", "[natural_unit]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.natural_unit0"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1000), power_ratio_type(2, 1)),
                        bpu<int64_t>(dim::mass, scalefactor_ratio_type(1, 1000), power_ratio_type(-1, 1))));

                static_assert(v.n_bpu() == 2);

                log && log(xtag("v.abbrev", v.abbrev()));

                static_assert(v.abbrev().size() > 0);
                static_assert(v.abbrev() == flatstring("mm^2.mg^-1"));
            }
        } /*TEST_CASE(natural_unit0)*/

        TEST_CASE("natural_unit1", "[natural_unit]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.natural_unit1"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu<int64_t>(dim::distance, scalefactor_ratio_type(1000, 1), power_ratio_type(2, 1))));

                static_assert(v.n_bpu() == 1);

                log && log(xtag("v.abbrev", v.abbrev()));

                static_assert(v.abbrev().size() > 0);
                static_assert(v.abbrev() == flatstring("km^2"));
            }
        } /*TEST_CASE(natural_unit1)*/

        TEST_CASE("natural_unit2", "[natural_unit]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.natural_unit2"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1)),
                        bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1), power_ratio_type(1, 1)),
                        bpu<int64_t>(dim::time, scalefactor_ratio_type(1, 1), power_ratio_type(-2, 1))));

                static_assert(v.n_bpu() == 3);

                log && log(xtag("v.abbrev", v.abbrev()));

                static_assert(v.abbrev().size() > 0);
                static_assert(v.abbrev() == flatstring("kg.m.s^-2"));
            }
        } /*TEST_CASE(natural_unit2)*/

        TEST_CASE("natural_unit3", "[natural_unit]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.natural_unit3"));
            //log && log("(A)", xtag("foo", foo));

            {
                constexpr natural_unit<int64_t> v
                    = (bpu_array_maker<int64_t>::make_bpu_array
                       (bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1)),
                        bpu<int64_t>(dim::distance, scalefactor_ratio_type(1, 1), power_ratio_type(1, 1))));

                static_assert(v.n_bpu() == 2);

                log && log(xtag("v.abbrev", v.abbrev()));

                static_assert(v.abbrev().size() > 0);
                static_assert(v.abbrev() == flatstring("kg.m"));

                {
                    natural_unit<int64_t> w = v;

                    nu_ratio_inplace(&w,
                                     bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1)));

                    log && log(xtag("w.abbrev", w.abbrev()));

                    REQUIRE(w.n_bpu() == 1);
                    REQUIRE(w[0].native_dim() == dim::distance);
                    REQUIRE(w.abbrev() == flatstring("m"));
                }

                {
                    constexpr natural_unit<int64_t> w
                        = (bpu_array_maker<int64_t>::make_bpu_array
                           (bpu<int64_t>(dim::mass, scalefactor_ratio_type(1000, 1), power_ratio_type(1, 1))));

                    static_assert(w.n_bpu() == 1);

                    log && log(xtag("w.abbrev", w.abbrev()));

                    constexpr auto rr = nu_ratio(v, w);

                    log && log(xtag("rr", rr));

                    REQUIRE(rr.natural_unit_.n_bpu() == 1);
                    REQUIRE(rr.natural_unit_[0].native_dim() == dim::distance);
                    REQUIRE(rr.natural_unit_.abbrev() == flatstring("m"));
                }

                {
                    constexpr natural_unit<int64_t> w
                        = (bpu_array_maker<int64_t>::make_bpu_array
                           (bpu<int64_t>(dim::time, scalefactor_ratio_type(1, 1), power_ratio_type(1, 1))));

                    static_assert(w.n_bpu() == 1);

                    log && log(xtag("w.abbrev", w.abbrev()));

                    constexpr auto rr = nu_ratio(v, w);

                    log && log(xtag("rr", rr));

                    REQUIRE(rr.natural_unit_.n_bpu() == 3);
                    REQUIRE(rr.natural_unit_[0].native_dim() == dim::mass);
                    REQUIRE(rr.natural_unit_[1].native_dim() == dim::distance);
                    REQUIRE(rr.natural_unit_[2].native_dim() == dim::time);
                    REQUIRE(rr.natural_unit_.abbrev() == flatstring("kg.m.s^-1"));
                }

                {
                    natural_unit<int64_t> w = v;

                    REQUIRE(w.n_bpu() == 2);
                    REQUIRE(w[0].native_dim() == dim::mass);

                    nu_ratio_inplace(&w,
                                     bpu<int64_t>(dim::time, scalefactor_ratio_type(1, 1), power_ratio_type(2, 1)));

                    REQUIRE(w.n_bpu() == 3);
                    REQUIRE(w[0].native_dim() == dim::mass);
                    REQUIRE(w[1].native_dim() == dim::distance);
                    REQUIRE(w[2].native_dim() == dim::time);

                    log && log(xtag("w.abbrev", w.abbrev()));

                    REQUIRE(w.n_bpu() == 3);
                    REQUIRE(w.abbrev() == flatstring("kg.m.s^-2"));
                }
            }
        } /*TEST_CASE(natural_unit3)*/

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

        TEST_CASE("scaled_unit1", "[scaled_unit1]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.scaled_unit1"));
            //log && log("(A)", xtag("foo", foo));

            constexpr auto ng = su2::nanogram;
            constexpr auto ng2 = ng / ng;

            log && log(xtag("ng", ng));
            log && log(xtag("ng/ng", ng2));
            //log && log(xtag("ng/ng",

            static_assert(ng.natural_unit_.n_bpu() == 1);
            static_assert(ng2.natural_unit_.n_bpu() == 0);
        } /*TEST_CASE(scaled_unit1)*/

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
            Quantity ng = unit_qty(su2::nanogram);
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
            Quantity ng = unit_qty(su2::nanogram);
            auto ng0 = ng / ng;

            log && log(xtag("ng/ng", ng0));

            REQUIRE(ng0.scale() == 1);
        } /*TEST_CASE(Quantity3)*/

        TEST_CASE("Quantity4", "[Quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.Quantity4"));
            //log && log("(A)", xtag("foo", foo));

            /* not constexpr until c++26 */
            Quantity ng = unit_qty(su2::nanogram);
            Quantity ug = unit_qty(su2::microgram);

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
        } /*TEST_CASE(Quantity4)*/

        TEST_CASE("Quantity5", "[Quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.Quantity5"));
            //log && log("(A)", xtag("foo", foo));

            /* not constexpr until c++26 */
            Quantity ng = unit_qty(su2::nanogram);
            Quantity ug = unit_qty(su2::microgram);

            {
                auto ratio1 = ng / ug;
                log && log(xtag("ng/ug", ratio1));

                /* units will be nanograms,  since that's on lhs */
                REQUIRE(ratio1.unit().n_bpu() == 0);
                REQUIRE(ratio1.scale() == 0.001);
            }

            {
                auto ratio2 = ug / ng;
                log && log(xtag("ug/ng", ratio2));

                REQUIRE(ratio2.unit().n_bpu() == 0);
                REQUIRE(ratio2.scale() == 1000.0);
            }

            //REQUIRE(ng2.scale() == 1);
        } /*TEST_CASE(Quantity5)*/

        TEST_CASE("Quantity6", "[Quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.Quantity6"));
            //log && log("(A)", xtag("foo", foo));

            /* not constexpr until c++26 */
            Quantity ng = unit_qty(su2::nanogram);
            Quantity ug = unit_qty(su2::microgram);

            {
                auto sum1 = ng + ug;
                log && log(xtag("ng+ug", sum1));

                /* units will be nanograms,  since that's on lhs */
                REQUIRE(sum1.unit().n_bpu() == 1);
                REQUIRE(sum1.unit()[0].scalefactor() == scalefactor_ratio_type(1, 1000000000));
                REQUIRE(sum1.scale() == 1001.0);
            }

            {
                auto sum2 = ug + ng;
                log && log(xtag("ug+ng", sum2));

                /* units will be micrograms,  since that's on rhs */
                REQUIRE(sum2.unit().n_bpu() == 1);
                REQUIRE(sum2.unit()[0].scalefactor() == scalefactor_ratio_type(1, 1000000));
                REQUIRE(sum2.scale() == 1.001);
            }

            //REQUIRE(ng2.scale() == 1);
        } /*TEST_CASE(Quantity6)*/

        TEST_CASE("Quantity7", "[Quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.Quantity7"));
            //log && log("(A)", xtag("foo", foo));

            /* not constexpr until c++26 */
            Quantity ng = unit_qty(su2::nanogram);
            Quantity ug = unit_qty(su2::microgram);

            {
                auto sum1 = ng - ug;
                log && log(xtag("ng-ug", sum1));

                /* units will be nanograms,  since that's on lhs */
                REQUIRE(sum1.unit().n_bpu() == 1);
                REQUIRE(sum1.unit()[0].scalefactor() == scalefactor_ratio_type(1, 1000000000));
                REQUIRE(sum1.scale() == -999.0);
            }

            {
                auto sum2 = ug - ng;
                log && log(xtag("ug-ng", sum2));

                /* units will be micrograms,  since that's on rhs */
                REQUIRE(sum2.unit().n_bpu() == 1);
                REQUIRE(sum2.unit()[0].scalefactor() == scalefactor_ratio_type(1, 1000000));
                REQUIRE(sum2.scale() == 0.999);
            }

            //REQUIRE(ng2.scale() == 1);
        } /*TEST_CASE(Quantity7)*/
    } /*namespace ut*/
} /*namespace xo*/

/* end dimension.test.cpp */
