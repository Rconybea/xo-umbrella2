/* @file bu.test.cpp */

#include "xo/unit/basis_unit.hpp"
#include "xo/indentlog/scope.hpp"
//#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::qty::scalefactor_ratio_type;
    using xo::qty::abbrev::basis_unit2_abbrev;
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
        constexpr bu_abbrev_type bu_mpl_abbrev = bu.abbrev();

        TEST_CASE("basis_unit", "[basis_unit]") {
            static_assert(bu_mpl_abbrev<bu::gram> == bu::gram.abbrev());
            REQUIRE(bu_mpl_abbrev<bu::gram> == bu::gram.abbrev());
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

            log && log(xtag("mass*10^3", basis_unit2_abbrev(dim::mass, scalefactor_ratio_type(1000, 1))));

            static_assert(basis_unit2_abbrev(dim::mass, scalefactor_ratio_type(1000, 1))
                          == bu_abbrev_type::from_chars("kg"));

            log && log("---------------------");

            /* note: using CHECK to make test show up in coverage */

#          define REQUIRE_x2(x) static_assert(x); REQUIRE(x)

            log && log(xtag("mass", basis_unit2_abbrev(dim::distance, scalefactor_ratio_type(1, 1))));

            REQUIRE_x2(bu::picogram.abbrev() == bu_abbrev_type::from_chars("pg"));
            REQUIRE_x2(bu::nanogram.abbrev()
                          == bu_abbrev_type::from_chars("ng"));
            REQUIRE_x2(bu::microgram.abbrev()
                          == bu_abbrev_type::from_chars("ug"));
            REQUIRE_x2(bu::milligram.abbrev()
                          == bu_abbrev_type::from_chars("mg"));
            REQUIRE_x2(bu::gram.abbrev()
                          == bu_abbrev_type::from_chars("g"));
            REQUIRE_x2(bu::kilogram.abbrev()
                          == bu_abbrev_type::from_chars("kg"));
            REQUIRE_x2(bu::tonne.abbrev()
                          == bu_abbrev_type::from_chars("t"));
            REQUIRE_x2(bu::kilotonne.abbrev()
                          == bu_abbrev_type::from_chars("kt"));
            REQUIRE_x2(bu::megatonne.abbrev()
                          == bu_abbrev_type::from_chars("Mt"));
            REQUIRE_x2(bu::gigatonne.abbrev()
                          == bu_abbrev_type::from_chars("Gt"));

            log && log(xtag("distance", basis_unit2_abbrev(dim::distance, scalefactor_ratio_type(1, 1))));

            REQUIRE_x2(bu::picometre.abbrev()
                          == bu_abbrev_type::from_chars("pm"));
            REQUIRE_x2(bu::nanometre.abbrev()
                          == bu_abbrev_type::from_chars("nm"));
            REQUIRE_x2(bu::micrometre.abbrev()
                          == bu_abbrev_type::from_chars("um"));
            REQUIRE_x2(bu::millimetre.abbrev()
                          == bu_abbrev_type::from_chars("mm"));
            REQUIRE_x2(bu::metre.abbrev()
                          == bu_abbrev_type::from_chars("m"));
            REQUIRE_x2(bu::kilometre.abbrev()
                          == bu_abbrev_type::from_chars("km"));
            REQUIRE_x2(bu::megametre.abbrev()
                          == bu_abbrev_type::from_chars("Mm"));
            REQUIRE_x2(bu::gigametre.abbrev()
                          == bu_abbrev_type::from_chars("Gm"));

            REQUIRE_x2(bu::picometer.abbrev()
                          == bu_abbrev_type::from_chars("pm"));
            REQUIRE_x2(bu::nanometer.abbrev()
                          == bu_abbrev_type::from_chars("nm"));
            REQUIRE_x2(bu::micrometer.abbrev()
                          == bu_abbrev_type::from_chars("um"));
            REQUIRE_x2(bu::millimeter.abbrev()
                          == bu_abbrev_type::from_chars("mm"));
            REQUIRE_x2(bu::meter.abbrev()
                          == bu_abbrev_type::from_chars("m"));
            REQUIRE_x2(bu::kilometer.abbrev()
                          == bu_abbrev_type::from_chars("km"));
            REQUIRE_x2(bu::megameter.abbrev()
                          == bu_abbrev_type::from_chars("Mm"));
            REQUIRE_x2(bu::gigameter.abbrev()
                          == bu_abbrev_type::from_chars("Gm"));

            REQUIRE_x2(bu::lightsecond.abbrev() == flatstring("lsec"));
            REQUIRE_x2(bu::astronomicalunit.abbrev() == flatstring("AU"));

            log && log(xtag("time", basis_unit2_abbrev(dim::time, scalefactor_ratio_type(1, 1))));

            REQUIRE_x2(bu::picosecond.abbrev() == bu_abbrev_type::from_chars("ps"));
            REQUIRE_x2(bu::nanosecond.abbrev() == bu_abbrev_type::from_chars("ns"));
            REQUIRE_x2(bu::microsecond.abbrev() == bu_abbrev_type::from_chars("us"));
            REQUIRE_x2(bu::millisecond.abbrev() == bu_abbrev_type::from_chars("ms"));
            REQUIRE_x2(bu::second.abbrev() == bu_abbrev_type::from_chars("s"));
            REQUIRE_x2(bu::minute.abbrev() == bu_abbrev_type::from_chars("min"));
            REQUIRE_x2(bu::hour.abbrev() == bu_abbrev_type::from_chars("hr"));
            REQUIRE_x2(bu::day.abbrev() == bu_abbrev_type::from_chars("dy"));
            REQUIRE_x2(bu::week.abbrev() == bu_abbrev_type::from_chars("wk"));
            REQUIRE_x2(bu::month.abbrev() == bu_abbrev_type::from_chars("mo"));

            REQUIRE_x2(bu::year.abbrev() == bu_abbrev_type::from_chars("yr"));
            REQUIRE_x2(bu::year250.abbrev() == bu_abbrev_type::from_chars("yr250"));
            REQUIRE_x2(bu::year360.abbrev() == bu_abbrev_type::from_chars("yr360"));
            REQUIRE_x2(bu::year365.abbrev() == bu_abbrev_type::from_chars("yr365"));

            log && log(xtag("currency", basis_unit2_abbrev(dim::currency, scalefactor_ratio_type(1, 1))));

            REQUIRE_x2(bu::currency.abbrev() == flatstring("ccy"));

            log && log(xtag("price", basis_unit2_abbrev(dim::price, scalefactor_ratio_type(1, 1))));

            REQUIRE_x2(bu::price.abbrev() == flatstring("px"));

#          undef REQUIRE_x2

#ifdef OBSOLETE
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
#endif

        } /*TEST_CASE(basis_unit1)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end bu.test.cpp */
