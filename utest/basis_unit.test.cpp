/* @file bu.test.cpp */

#include "xo/unit/basis_unit.hpp"
#include "xo/unit/bpu_store.hpp"
#include "xo/indentlog/scope.hpp"
//#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::qty::scalefactor_ratio_type;
    using xo::qty::bu_abbrev;
    using xo::qty::basis_unit;
    using xo::qty::bu_abbrev_type;
    using xo::qty::native_unit2_v;
    using xo::qty::dim;
    namespace bu = xo::qty::detail::bu;

    namespace ut {

        /* compile-time test:
         * verify we can use a basis_unit as a non-type template parameter
         * we will need this for quantity<Repr, Int, natural_unit<Int>>
         */
        template <basis_unit bu>
        constexpr bu_abbrev_type bu_mpl_abbrev = bu_abbrev(bu);

        TEST_CASE("basis_unit", "[basis_unit]") {
            static_assert(bu_mpl_abbrev<bu::gram> == bu_abbrev(bu::gram));
            REQUIRE(bu_mpl_abbrev<bu::gram> == bu_abbrev(bu::gram));
        } /*TEST_CASE(basis_unit)*/

        TEST_CASE("basis_unit1", "[basis_unit]") {
            constexpr bool c_debug_flag = false;
            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.basis_unit1"));

            static_assert(native_unit2_v[static_cast<int>(dim::mass)].native_dim() == dim::mass);
            static_assert(native_unit2_v[static_cast<int>(dim::distance)].native_dim() == dim::distance);
            static_assert(native_unit2_v[static_cast<int>(dim::time)].native_dim() == dim::time);
            static_assert(native_unit2_v[static_cast<int>(dim::time)].native_dim() == dim::time);
            static_assert(native_unit2_v[static_cast<int>(dim::currency)].native_dim() == dim::currency);
            static_assert(native_unit2_v[static_cast<int>(dim::price)].native_dim() == dim::price);

            log && log(xtag("mass*10^3", bu_abbrev(bu::kilogram)));

            static_assert(bu_abbrev(bu::kilogram)
                          == bu_abbrev_type::from_chars("kg"));

            log && log("---------------------");

            /* note: using CHECK to make test show up in coverage */

#          define REQUIRE_x2(x) static_assert(x); REQUIRE(x)

            log && log(xtag("mass", bu_abbrev(bu::meter)));

            REQUIRE_x2(bu_abbrev(bu::picogram) == bu_abbrev_type::from_chars("pg"));
            REQUIRE_x2(bu_abbrev(bu::nanogram)
                          == bu_abbrev_type::from_chars("ng"));
            REQUIRE_x2(bu_abbrev(bu::microgram)
                          == bu_abbrev_type::from_chars("ug"));
            REQUIRE_x2(bu_abbrev(bu::milligram)
                          == bu_abbrev_type::from_chars("mg"));
            REQUIRE_x2(bu_abbrev(bu::gram)
                          == bu_abbrev_type::from_chars("g"));
            REQUIRE_x2(bu_abbrev(bu::kilogram)
                          == bu_abbrev_type::from_chars("kg"));
            REQUIRE_x2(bu_abbrev(bu::tonne)
                          == bu_abbrev_type::from_chars("t"));
            REQUIRE_x2(bu_abbrev(bu::kilotonne)
                          == bu_abbrev_type::from_chars("kt"));
            REQUIRE_x2(bu_abbrev(bu::megatonne)
                          == bu_abbrev_type::from_chars("Mt"));
            REQUIRE_x2(bu_abbrev(bu::gigatonne)
                          == bu_abbrev_type::from_chars("Gt"));

            log && log(xtag("distance", bu_abbrev(bu::meter)));

            REQUIRE_x2(bu_abbrev(bu::picometre)
                          == bu_abbrev_type::from_chars("pm"));
            REQUIRE_x2(bu_abbrev(bu::nanometre)
                          == bu_abbrev_type::from_chars("nm"));
            REQUIRE_x2(bu_abbrev(bu::micrometre)
                          == bu_abbrev_type::from_chars("um"));
            REQUIRE_x2(bu_abbrev(bu::millimetre)
                          == bu_abbrev_type::from_chars("mm"));
            REQUIRE_x2(bu_abbrev(bu::metre)
                          == bu_abbrev_type::from_chars("m"));
            REQUIRE_x2(bu_abbrev(bu::kilometre)
                          == bu_abbrev_type::from_chars("km"));
            REQUIRE_x2(bu_abbrev(bu::megametre)
                          == bu_abbrev_type::from_chars("Mm"));
            REQUIRE_x2(bu_abbrev(bu::gigametre)
                          == bu_abbrev_type::from_chars("Gm"));

            REQUIRE_x2(bu_abbrev(bu::picometer)
                          == bu_abbrev_type::from_chars("pm"));
            REQUIRE_x2(bu_abbrev(bu::nanometer)
                          == bu_abbrev_type::from_chars("nm"));
            REQUIRE_x2(bu_abbrev(bu::micrometer)
                          == bu_abbrev_type::from_chars("um"));
            REQUIRE_x2(bu_abbrev(bu::millimeter)
                          == bu_abbrev_type::from_chars("mm"));
            REQUIRE_x2(bu_abbrev(bu::meter)
                          == bu_abbrev_type::from_chars("m"));
            REQUIRE_x2(bu_abbrev(bu::kilometer)
                          == bu_abbrev_type::from_chars("km"));
            REQUIRE_x2(bu_abbrev(bu::megameter)
                          == bu_abbrev_type::from_chars("Mm"));
            REQUIRE_x2(bu_abbrev(bu::gigameter)
                          == bu_abbrev_type::from_chars("Gm"));

            REQUIRE_x2(bu_abbrev(bu::lightsecond) == flatstring("lsec"));
            REQUIRE_x2(bu_abbrev(bu::astronomicalunit) == flatstring("AU"));

            REQUIRE_x2(bu_abbrev(bu::inch) == flatstring("in"));
            REQUIRE_x2(bu_abbrev(bu::foot) == flatstring("ft"));
            REQUIRE_x2(bu_abbrev(bu::yard) == flatstring("yd"));
            REQUIRE_x2(bu_abbrev(bu::mile) == flatstring("mi"));

            log && log(xtag("time", bu_abbrev(bu::second)));

            REQUIRE_x2(bu_abbrev(bu::picosecond) == bu_abbrev_type::from_chars("ps"));
            REQUIRE_x2(bu_abbrev(bu::nanosecond) == bu_abbrev_type::from_chars("ns"));
            REQUIRE_x2(bu_abbrev(bu::microsecond) == bu_abbrev_type::from_chars("us"));
            REQUIRE_x2(bu_abbrev(bu::millisecond) == bu_abbrev_type::from_chars("ms"));
            REQUIRE_x2(bu_abbrev(bu::second) == bu_abbrev_type::from_chars("s"));
            REQUIRE_x2(bu_abbrev(bu::minute) == bu_abbrev_type::from_chars("min"));
            REQUIRE_x2(bu_abbrev(bu::hour) == bu_abbrev_type::from_chars("hr"));
            REQUIRE_x2(bu_abbrev(bu::day) == bu_abbrev_type::from_chars("dy"));
            REQUIRE_x2(bu_abbrev(bu::week) == bu_abbrev_type::from_chars("wk"));
            REQUIRE_x2(bu_abbrev(bu::month) == bu_abbrev_type::from_chars("mo"));

            REQUIRE_x2(bu_abbrev(bu::year) == bu_abbrev_type::from_chars("yr"));
            REQUIRE_x2(bu_abbrev(bu::year250) == bu_abbrev_type::from_chars("yr250"));
            REQUIRE_x2(bu_abbrev(bu::year360) == bu_abbrev_type::from_chars("yr360"));
            REQUIRE_x2(bu_abbrev(bu::year365) == bu_abbrev_type::from_chars("yr365"));

            log && log(xtag("currency", bu_abbrev(bu::currency)));

            REQUIRE_x2(bu_abbrev(bu::currency) == flatstring("ccy"));

            log && log(xtag("price", bu_abbrev(bu::price)));

            REQUIRE_x2(bu_abbrev(bu::price) == flatstring("px"));

#          undef REQUIRE_x2

        } /*TEST_CASE(basis_unit1)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end bu.test.cpp */
