/* @file unit.test.cpp */

#include "xo/unit/scaled_unit_iostream.hpp"
//#include "xo/unit/scaled_unit.hpp"
#include "xo/unit/bu_store.hpp"
#include "xo/unit/bpu.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace ut {
        /* compile-time tests */

        namespace bu = xo::qty::detail::bu;
        using xo::qty::bu_abbrev_store;
        using xo::qty::bu_abbrev_type;
        using xo::qty::scalefactor_ratio_type;
        using xo::qty::power_ratio_type;
        using xo::qty::dim;

        TEST_CASE("basis_unit2_store", "[basis_unit2_store]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.basis_unit2_store"));
            //log && log("(A)", xtag("foo", foo));

            log && log(xtag("mass*10^-9",       bu_abbrev_store.bu_abbrev(bu::picogram)));
            log && log(xtag("mass*10^-6",       bu_abbrev_store.bu_abbrev(bu::microgram)));
            log && log(xtag("mass*10^-3",       bu_abbrev_store.bu_abbrev(bu::milligram)));
            log && log(xtag("mass",             bu_abbrev_store.bu_abbrev(bu::gram)));
            log && log(xtag("mass*10^3",        bu_abbrev_store.bu_abbrev(bu::kilogram)));
            log && log(xtag("mass*10^6",        bu_abbrev_store.bu_abbrev(bu::tonne)));
            log && log(xtag("mass*10^9",        bu_abbrev_store.bu_abbrev(bu::megatonne)));

            log && log(xtag("distance*10^-9",   bu_abbrev_store.bu_abbrev(bu::nanometer)));
            log && log(xtag("distance*10^-6",   bu_abbrev_store.bu_abbrev(bu::micrometer)));
            log && log(xtag("distance*10^-3",   bu_abbrev_store.bu_abbrev(bu::millimeter)));
            log && log(xtag("distance",         bu_abbrev_store.bu_abbrev(bu::meter)));
            log && log(xtag("distance*10^3",    bu_abbrev_store.bu_abbrev(bu::kilometer)));

            log && log(xtag("time*10^-9",       bu_abbrev_store.bu_abbrev(bu::nanosecond)));
            log && log(xtag("time*10^-6",       bu_abbrev_store.bu_abbrev(bu::microsecond)));
            log && log(xtag("time*10^-3",       bu_abbrev_store.bu_abbrev(bu::millisecond)));
            log && log(xtag("time",             bu_abbrev_store.bu_abbrev(bu::second)));
            log && log(xtag("time*60",          bu_abbrev_store.bu_abbrev(bu::minute)));
            log && log(xtag("time*3600",        bu_abbrev_store.bu_abbrev(bu::hour)));
            log && log(xtag("time*24*3600",     bu_abbrev_store.bu_abbrev(bu::day)));
            log && log(xtag("time*250*24*3600", bu_abbrev_store.bu_abbrev(bu::year250)));
            log && log(xtag("time*360*24*3600", bu_abbrev_store.bu_abbrev(bu::year360)));
            log && log(xtag("time*365*24*3600", bu_abbrev_store.bu_abbrev(bu::year365)));

            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::nanogram),    "ng") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::microgram),   "ug") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::milligram),   "mg") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::gram),        "g") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::kilogram),    "kg") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::tonne),       "t") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::kilotonne),   "kt") == 0);

            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::nanometer),   "nm") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::micrometer),  "um") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::millimeter),  "mm") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::meter),       "m") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::kilometer),   "km") == 0);

            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::nanosecond),  "ns") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::microsecond), "us") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::millisecond), "ms") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::second),      "s") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::minute),      "min") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::hour),        "hr") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::day),         "dy") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::year250),     "yr250") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::year360),     "yr360") == 0);
            REQUIRE(::strcmp(bu_abbrev_store.bu_abbrev(bu::year365),     "yr365") == 0);

        } /*TEST_CASE(basis_unit2_store)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end dimension.test.cpp */
