/* @file unit.test.cpp */

#include "xo/unit/scaled_unit_iostream.hpp"
//#include "xo/unit/scaled_unit.hpp"
#include "xo/unit/bpu_store.hpp"
#include "xo/unit/bpu.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace ut {
        /* compile-time tests */

        namespace su = xo::qty::su;

        using xo::qty::dim;
        using xo::qty::basis_unit2_abbrev_type;
        using xo::qty::scalefactor_ratio_type;
        using xo::qty::basis_unit2_store;
        using xo::qty::power_ratio_type;

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

    } /*namespace ut*/
} /*namespace xo*/

/* end dimension.test.cpp */
