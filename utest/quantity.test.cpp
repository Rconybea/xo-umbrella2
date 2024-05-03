/* @file quantity.test.cpp */

#include "xo/unit/quantity.hpp"
#include "xo/unit/quantity_iostream.hpp"
#include "xo/unit/quantity_concept.hpp"
#include "xo/indentlog/scope.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace qty {
        TEST_CASE("quantity.mass", "[quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.quantity.mass"));
            //log && log("(A)", xtag("foo", foo));

            constexpr auto pg = qty::picograms(1.0);
            static_assert(quantity_concept<decltype(pg)>);
            static_assert(sizeof(pg) == sizeof(double));
            static_assert(pg.scale() == 1.0);
            static_assert(pg.abbrev() == flatstring("pg"));

            constexpr auto ng = qty::nanograms(1.0);
            static_assert(quantity_concept<decltype(ng)>);
            static_assert(sizeof(ng) == sizeof(double));
            static_assert(ng.scale() == 1.0);
            static_assert(ng.abbrev() == flatstring("ng"));

            constexpr auto ug = qty::micrograms(1.0);
            static_assert(quantity_concept<decltype(ug)>);
            static_assert(sizeof(ug) == sizeof(double));
            static_assert(ug.scale() == 1.0);
            static_assert(ug.abbrev() == flatstring("ug"));

            constexpr auto mg = qty::milligrams(1.0);
            static_assert(quantity_concept<decltype(mg)>);
            static_assert(sizeof(mg) == sizeof(double));
            static_assert(mg.scale() == 1.0);
            static_assert(mg.abbrev() == flatstring("mg"));

            constexpr auto g = qty::grams(1.0);
            static_assert(quantity_concept<decltype(g)>);
            static_assert(sizeof(g) == sizeof(double));
            static_assert(g.scale() == 1.0);
            static_assert(g.abbrev() == flatstring("g"));

            constexpr auto kg = qty::kilograms(1.0);
            static_assert(quantity_concept<decltype(kg)>);
            static_assert(sizeof(kg) == sizeof(double));
            static_assert(kg.scale() == 1.0);
            static_assert(kg.abbrev() == flatstring("kg"));

            constexpr auto t = qty::tonnes(1.0);
            static_assert(quantity_concept<decltype(t)>);
            static_assert(sizeof(t) == sizeof(double));
            static_assert(t.scale() == 1.0);
            static_assert(t.abbrev() == flatstring("t"));

            constexpr auto kt = qty::kilotonnes(1.0);
            static_assert(quantity_concept<decltype(kt)>);
            static_assert(sizeof(kt) == sizeof(double));
            static_assert(kt.scale() == 1.0);
            static_assert(kt.abbrev() == flatstring("kt"));

            constexpr auto mt = qty::megatonnes(1.0);
            static_assert(quantity_concept<decltype(mt)>);
            static_assert(sizeof(mt) == sizeof(double));
            static_assert(mt.scale() == 1.0);
            static_assert(mt.abbrev() == flatstring("Mt"));

            constexpr auto gt = qty::gigatonnes(1.0);
            static_assert(quantity_concept<decltype(gt)>);
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
        } /*TEST_CASE(quantity.mass)*/

        TEST_CASE("quantity.distance", "[quantity]") {
            constexpr bool c_debug_flag = true;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.quantity.distance"));

            constexpr auto pm = qty::picometers(1.0);
            static_assert(quantity_concept<decltype(pm)>);
            static_assert(sizeof(pm) == sizeof(double));
            static_assert(pm.scale() == 1.0);
            static_assert(pm.abbrev() == flatstring("pm"));

            constexpr auto nm = qty::nanometers(1.0);
            static_assert(quantity_concept<decltype(nm)>);
            static_assert(sizeof(nm) == sizeof(double));
            static_assert(nm.scale() == 1.0);
            static_assert(nm.abbrev() == flatstring("nm"));

            constexpr auto um = qty::micrometers(1.0);
            static_assert(quantity_concept<decltype(um)>);
            static_assert(sizeof(um) == sizeof(double));
            static_assert(um.scale() == 1.0);
            static_assert(um.abbrev() == flatstring("um"));

            constexpr auto mm = qty::millimeters(1.0);
            static_assert(quantity_concept<decltype(mm)>);
            static_assert(sizeof(mm) == sizeof(double));
            static_assert(mm.scale() == 1.0);
            static_assert(mm.abbrev() == flatstring("mm"));

            constexpr auto m = qty::meters(1.0);
            static_assert(quantity_concept<decltype(m)>);
            static_assert(sizeof(m) == sizeof(double));
            static_assert(m.scale() == 1.0);
            static_assert(m.abbrev() == flatstring("m"));

            constexpr auto km = qty::kilometers(1.0);
            static_assert(quantity_concept<decltype(km)>);
            static_assert(sizeof(km) == sizeof(double));
            static_assert(km.scale() == 1.0);
            static_assert(km.abbrev() == flatstring("km"));

            constexpr auto Mm = qty::megameters(1.0);
            static_assert(quantity_concept<decltype(Mm)>);
            static_assert(sizeof(Mm) == sizeof(double));
            static_assert(Mm.scale() == 1.0);
            static_assert(Mm.abbrev() == flatstring("Mm"));

            constexpr auto Gm = qty::gigameters(1.0);
            static_assert(quantity_concept<decltype(Gm)>);
            static_assert(sizeof(Gm) == sizeof(double));
            static_assert(Gm.scale() == 1.0);
            static_assert(Gm.abbrev() == flatstring("Gm"));

            constexpr auto lsec = qty::lightseconds(1.0);
            static_assert(quantity_concept<decltype(lsec)>);
            static_assert(sizeof(lsec) == sizeof(double));
            static_assert(lsec.scale() == 1.0);
            static_assert(lsec.abbrev() == flatstring("lsec"));

            constexpr auto AU = qty::astronomicalunits(1.0);
            static_assert(quantity_concept<decltype(AU)>);
            static_assert(sizeof(AU) == sizeof(double));
            static_assert(AU.scale() == 1.0);
            static_assert(AU.abbrev() == flatstring("AU"));

            log && log(xtag("pm.abbrev", pm.abbrev()));
            log && log(xtag("nm.abbrev", nm.abbrev()));
            log && log(xtag("um.abbrev", um.abbrev()));
            log && log(xtag("mm.abbrev", mm.abbrev()));
            log && log(xtag("m.abbrev", m.abbrev()));
            log && log(xtag("km.abbrev", km.abbrev()));
            log && log(xtag("Mm.abbrev", Mm.abbrev()));
            log && log(xtag("Gm.abbrev", Gm.abbrev()));
            log && log(xtag("lsec.abbrev", lsec.abbrev()));
            log && log(xtag("AU.abbrev", AU.abbrev()));

            REQUIRE(tostr(pm) == "1pm");
            REQUIRE(tostr(nm) == "1nm");
            REQUIRE(tostr(um) == "1um");
            REQUIRE(tostr(mm) == "1mm");
            REQUIRE(tostr(m) == "1m");
            REQUIRE(tostr(km) == "1km");
            REQUIRE(tostr(Mm) == "1Mm");
            REQUIRE(tostr(Gm) == "1Gm");
            REQUIRE(tostr(lsec) == "1lsec");
            REQUIRE(tostr(AU) == "1AU");

        } /*TEST_CASE(quantity.distance)*/

        TEST_CASE("quantity.time", "[quantity]") {
            constexpr bool c_debug_flag = true;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.quantity.time"));

            constexpr auto ps = qty::picoseconds(1.0);
            static_assert(quantity_concept<decltype(ps)>);
            static_assert(sizeof(ps) == sizeof(double));
            static_assert(ps.scale() == 1.0);
            static_assert(ps.abbrev() == flatstring("ps"));

            constexpr auto ns = qty::nanoseconds(1.0);
            static_assert(quantity_concept<decltype(ns)>);
            static_assert(sizeof(ns) == sizeof(double));
            static_assert(ns.scale() == 1.0);
            static_assert(ns.abbrev() == flatstring("ns"));

            constexpr auto us = qty::microseconds(1.0);
            static_assert(quantity_concept<decltype(us)>);
            static_assert(sizeof(us) == sizeof(double));
            static_assert(us.scale() == 1.0);
            static_assert(us.abbrev() == flatstring("us"));

            constexpr auto ms = qty::milliseconds(1.0);
            static_assert(quantity_concept<decltype(ms)>);
            static_assert(sizeof(ms) == sizeof(double));
            static_assert(ms.scale() == 1.0);
            static_assert(ms.abbrev() == flatstring("ms"));

            constexpr auto s = qty::seconds(1.0);
            static_assert(quantity_concept<decltype(s)>);
            static_assert(sizeof(s) == sizeof(double));
            static_assert(s.scale() == 1.0);
            static_assert(s.abbrev() == flatstring("s"));

            constexpr auto min = qty::minutes(1.0);
            static_assert(quantity_concept<decltype(min)>);
            static_assert(sizeof(min) == sizeof(double));
            static_assert(min.scale() == 1.0);
            static_assert(min.abbrev() == flatstring("min"));

            constexpr auto hr = qty::hours(1.0);
            static_assert(quantity_concept<decltype(hr)>);
            static_assert(sizeof(hr) == sizeof(double));
            static_assert(hr.scale() == 1.0);
            static_assert(hr.abbrev() == flatstring("hr"));

            constexpr auto dy = qty::days(1.0);
            static_assert(quantity_concept<decltype(dy)>);
            static_assert(sizeof(dy) == sizeof(double));
            static_assert(dy.scale() == 1.0);
            static_assert(dy.abbrev() == flatstring("dy"));

            constexpr auto wk = qty::weeks(1.0);
            static_assert(quantity_concept<decltype(wk)>);
            static_assert(sizeof(wk) == sizeof(double));
            static_assert(wk.scale() == 1.0);
            static_assert(wk.abbrev() == flatstring("wk"));

            constexpr auto mo = qty::months(1.0);
            static_assert(quantity_concept<decltype(mo)>);
            static_assert(sizeof(mo) == sizeof(double));
            static_assert(mo.scale() == 1.0);
            static_assert(mo.abbrev() == flatstring("mo"));

            constexpr auto yr = qty::years(1.0);
            static_assert(quantity_concept<decltype(yr)>);
            static_assert(sizeof(yr) == sizeof(double));
            static_assert(yr.scale() == 1.0);
            static_assert(yr.abbrev() == flatstring("yr"));

            constexpr auto yr250 = qty::year250s(1.0);
            static_assert(quantity_concept<decltype(yr250)>);
            static_assert(sizeof(yr250) == sizeof(double));
            static_assert(yr250.scale() == 1.0);
            static_assert(yr250.abbrev() == flatstring("yr250"));

            constexpr auto yr360 = qty::year360s(1.0);
            static_assert(quantity_concept<decltype(yr360)>);
            static_assert(sizeof(yr360) == sizeof(double));
            static_assert(yr360.scale() == 1.0);
            static_assert(yr360.abbrev() == flatstring("yr360"));

            constexpr auto yr365 = qty::year365s(1.0);
            static_assert(quantity_concept<decltype(yr365)>);
            static_assert(sizeof(yr365) == sizeof(double));
            static_assert(yr365.scale() == 1.0);
            static_assert(yr365.abbrev() == flatstring("yr365"));

            log && log(xtag("ps.abbrev", ps.abbrev()));
            log && log(xtag("ns.abbrev", ns.abbrev()));
            log && log(xtag("us.abbrev", us.abbrev()));
            log && log(xtag("ms.abbrev", ms.abbrev()));
            log && log(xtag("s.abbrev", s.abbrev()));
            log && log(xtag("min.abbrev", min.abbrev()));
            log && log(xtag("hr.abbrev", hr.abbrev()));
            log && log(xtag("dy.abbrev", dy.abbrev()));
            log && log(xtag("wk.abbrev", wk.abbrev()));
            log && log(xtag("mo.abbrev", mo.abbrev()));
            log && log(xtag("yr.abbrev", yr.abbrev()));
            log && log(xtag("yr250.abbrev", yr250.abbrev()));
            log && log(xtag("yr360.abbrev", yr360.abbrev()));
            log && log(xtag("yr365.abbrev", yr365.abbrev()));

            REQUIRE(tostr(ps) == "1ps");
            REQUIRE(tostr(ns) == "1ns");
            REQUIRE(tostr(us) == "1us");
            REQUIRE(tostr(ms) == "1ms");
            REQUIRE(tostr(s) == "1s");
            REQUIRE(tostr(min) == "1min");
            REQUIRE(tostr(hr) == "1hr");
            REQUIRE(tostr(dy) == "1dy");
            REQUIRE(tostr(wk) == "1wk");
            REQUIRE(tostr(mo) == "1mo");
            REQUIRE(tostr(yr) == "1yr");
            REQUIRE(tostr(yr250) == "1yr250");
            REQUIRE(tostr(yr360) == "1yr360");
            REQUIRE(tostr(yr365) == "1yr365");
        } /*TEST_CASE(quantity.time)*/

        TEST_CASE("quantity.mult", "[quantity.mult]") {
            constexpr auto pg = qty::picograms(1.0);
            constexpr auto ng = qty::nanograms(1.0);
            constexpr auto ug = qty::micrograms(1.0);

            constexpr auto ng_in_pg = ng.rescale<pg.unit()>();

            static_assert(ng_in_pg.scale() == 1000);
            static_assert(ng_in_pg == pg * 1000);

            /* multiplication by dimensionless values is constexpr in c++23
             * comparison in not constexpr until c++26
             */

            /* picograms:nanograms */

            static_assert(pg * 1000 == ng);
            REQUIRE(pg * 1000 == ng);
            static_assert(1000 * pg == ng);
            REQUIRE(1000 * pg == ng);
            static_assert(ng * 0.001 == pg);
            REQUIRE(ng * 0.001 == pg);
            static_assert(0.001 * ng == pg);
            REQUIRE(0.001 * ng == pg);

            /* picograms:micrograms */

            static_assert(pg * 1e6 == ug);
            REQUIRE(pg * 1e6 == ug);
            static_assert(1e6 * pg == ug);
            REQUIRE(1e6 * pg == ug);
            static_assert(ug * 1e-6 == pg);
            REQUIRE(ug * 1e-6 == pg);
            static_assert(1e-6 * ug == pg);
            REQUIRE(1e-6 * ug == pg);

            /* nanograms:micrograms */

            static_assert(ng * 1e3 == ug);
            REQUIRE(ng * 1e3 == ug);
            static_assert(1e3 * ng == ug);
            REQUIRE(1e3 * ng == ug);
            static_assert(ug * 1e-3 == ng);
            REQUIRE(ug * 1e-3 == ng);
            static_assert(1e-3 * ug == ng);
            REQUIRE(1e-3 * ug == ng);

            // /* picograms:milligrams */
            // /* nanograms:milligrams */
            // /* micrograms:milligrams */

            // /* picograms:grams */
            // /* nanograms:grams */
            // /* micrograms:grams */
            // /* milligrams:grams */
        } /*TEST_CASE(quantity.mult)*/

    } /*namespace qty*/
} /*namespace xo*/

/* end quantity.test.cpp */
