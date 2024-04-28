/* @file quantity.test.cpp */

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"
#include "xo/unit/quantity2_concept.hpp"
#include "xo/indentlog/scope.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace qty {
        TEST_CASE("quantity", "[quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.quantity"));
            //log && log("(A)", xtag("foo", foo));

            constexpr auto pg = qty::picograms(1.0);
            static_assert(quantity2_concept<decltype(pg)>);
            static_assert(sizeof(pg) == sizeof(double));
            static_assert(pg.scale() == 1.0);
            static_assert(pg.abbrev() == flatstring("pg"));

            constexpr auto ng = qty::nanograms(1.0);
            static_assert(quantity2_concept<decltype(ng)>);
            static_assert(sizeof(ng) == sizeof(double));
            static_assert(ng.scale() == 1.0);
            static_assert(ng.abbrev() == flatstring("ng"));

            constexpr auto ug = qty::micrograms(1.0);
            static_assert(quantity2_concept<decltype(ug)>);
            static_assert(sizeof(ug) == sizeof(double));
            static_assert(ug.scale() == 1.0);
            static_assert(ug.abbrev() == flatstring("ug"));

            constexpr auto mg = qty::milligrams(1.0);
            static_assert(quantity2_concept<decltype(mg)>);
            static_assert(sizeof(mg) == sizeof(double));
            static_assert(mg.scale() == 1.0);
            static_assert(mg.abbrev() == flatstring("mg"));

            constexpr auto g = qty::grams(1.0);
            static_assert(quantity2_concept<decltype(g)>);
            static_assert(sizeof(g) == sizeof(double));
            static_assert(g.scale() == 1.0);
            static_assert(g.abbrev() == flatstring("g"));

            constexpr auto kg = qty::kilograms(1.0);
            static_assert(quantity2_concept<decltype(kg)>);
            static_assert(sizeof(kg) == sizeof(double));
            static_assert(kg.scale() == 1.0);
            static_assert(kg.abbrev() == flatstring("kg"));

            constexpr auto t = qty::tonnes(1.0);
            static_assert(quantity2_concept<decltype(t)>);
            static_assert(sizeof(t) == sizeof(double));
            static_assert(t.scale() == 1.0);
            static_assert(t.abbrev() == flatstring("t"));

            constexpr auto kt = qty::kilotonnes(1.0);
            static_assert(quantity2_concept<decltype(kt)>);
            static_assert(sizeof(kt) == sizeof(double));
            static_assert(kt.scale() == 1.0);
            static_assert(kt.abbrev() == flatstring("kt"));

            constexpr auto mt = qty::megatonnes(1.0);
            static_assert(quantity2_concept<decltype(mt)>);
            static_assert(sizeof(mt) == sizeof(double));
            static_assert(mt.scale() == 1.0);
            static_assert(mt.abbrev() == flatstring("Mt"));

            constexpr auto gt = qty::gigatonnes(1.0);
            static_assert(quantity2_concept<decltype(gt)>);
            static_assert(sizeof(gt) == sizeof(double));
            static_assert(gt.scale() == 1.0);
            static_assert(gt.abbrev() == flatstring("Gt"));

            log && log(xtag("pg.abbrev", pg.abbrev()));
            log && log(xtag("ng.abbrev", ng.abbrev()));
            log && log(xtag("ug.abbrev", ug.abbrev()));
            log && log(xtag("mg.abbrev", mg.abbrev()));
            log && log(xtag("g.abbrev", g.abbrev()));
            log && log(xtag("kg.abbrev", kg.abbrev()));
            log && log(xtag("t.abbrev", t.abbrev()));
            log && log(xtag("kt.abbrev", kt.abbrev()));
            log && log(xtag("mt.abbrev", mt.abbrev()));
            log && log(xtag("gt.abbrev", gt.abbrev()));

            log && log(xtag("pg", pg));

            REQUIRE(tostr(pg) == "1pg");
            REQUIRE(tostr(ng) == "1ng");
            REQUIRE(tostr(ug) == "1ug");
            REQUIRE(tostr(mg) == "1mg");
            REQUIRE(tostr(g) == "1g");
            REQUIRE(tostr(kg) == "1kg");
            REQUIRE(tostr(t) == "1t");
            REQUIRE(tostr(kt) == "1kt");
            REQUIRE(tostr(mt) == "1Mt");
            REQUIRE(tostr(gt) == "1Gt");
        } /*TEST_CASE(quantity)*/
    } /*namespace qty*/
} /*namespace xo*/

/* end quantity.test.cpp */
