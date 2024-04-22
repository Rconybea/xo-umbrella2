/* @file quantity.test.cpp */

#include "xo/unit/mpl/quantity.hpp"
#include "xo/reflect/Reflect.hpp"
//#include <xo/randomgen/random_seed.hpp>
//#include <xo/randomgen/xoshiro256.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <catch2/catch.hpp>
#include <compare>

namespace xo {
    using xo::unit::quantity;

    using xo::unit::qty::kilograms;

    using xo::unit::qty::meters;
    using xo::unit::qty::kilometers;

    using xo::unit::qty::milliseconds;
    using xo::unit::qty::seconds;
    using xo::unit::qty::minutes;
    using xo::unit::qty::hours;
    using xo::unit::qty::volatility30d;
    using xo::unit::qty::volatility250d;

    using xo::unit::unit_find_bpu_t;
    using xo::unit::unit_conversion_factor_t;
    using xo::unit::unit_cartesian_product_t;
    using xo::unit::unit_cartesian_product;
    using xo::unit::unit_invert_t;
    using xo::unit::unit_abbrev_v;
    using xo::unit::same_dimension_v;
    using xo::unit::dim;

    using xo::unit::from_ratio;
    using xo::unit::stringliteral_from_ratio;
    using xo::unit::ratio2str_aux;
    using xo::unit::cstr_from_ratio;

    using xo::reflect::Reflect;

    namespace units = xo::unit::units;

    namespace ut {
        /* use 'testcase' snippet to add test cases here */
        TEST_CASE("quantity", "[quantity]") {
            //constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            //scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.quantity"), xtag("foo", foo), ...);
            //log && log("(A)", xtag("foo", foo));

            quantity<units::second, int64_t> t = seconds(1L);

            REQUIRE(t.scale() == 1);

            static_assert(t.basis_power<dim::time> == 1);
            static_assert(t.basis_power<dim::mass> == 0);
        } /*TEST_CASE(quantity)*/

        TEST_CASE("add1", "[quantity]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.add1"));

            quantity<units::second, int64_t> t1 = seconds(1);
            quantity<units::second, int64_t> t2 = seconds(2);

            static_assert(std::same_as<decltype(t1)::unit_type, units::second>);
            static_assert(std::same_as<decltype(t2)::unit_type, units::second>);

            auto sum = t1 + t2;

            CHECK(strcmp(sum.unit_cstr(), "s") == 0);

            static_assert(std::same_as<decltype(sum)::unit_type, units::second>);
            static_assert(t1.basis_power<dim::time> == 1);
            static_assert(t2.basis_power<dim::time> == 1);

            REQUIRE(sum.scale() == 3);

        } /*TEST_CASE(add1)*/

        TEST_CASE("add2", "[quantity]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.add2"));

            quantity<units::second, int64_t> t1 = seconds(1);
            {
                CHECK(strcmp(t1.unit_cstr(), "s") == 0);
                CHECK(t1.scale() == 1);
            }

            auto m2 = minutes(2);

            {
                static_assert(m2.basis_power<dim::time> == 1);

                log && log(xtag("m2.scale", m2.scale()), xtag("m2.unit", m2.unit_cstr()));

                CHECK(m2.scale() == 2);
                CHECK(strcmp(m2.unit_cstr(), "min") == 0);
            }

            {
                auto m2_sec = m2.in_units_of<units::second, int64_t>();

                static_assert(std::same_as<decltype(m2_sec), int64_t>);

                log && log(XTAG(m2_sec));

                CHECK(m2_sec == 120);
            }

            quantity<units::second, int64_t> t2 = m2;
            {
                auto sum = t1 + t2;

                static_assert(std::same_as<decltype(sum)::unit_type, units::second>);
                static_assert(sum.basis_power<dim::time> == 1);

                log && log(xtag("t1.unit", t1.unit_cstr()), xtag("t2.unit", t2.unit_cstr()));
                log && log(xtag("sum.unit", sum.unit_cstr()));

                CHECK(strcmp(t2.unit_cstr(), "s") == 0);
                CHECK(strcmp(sum.unit_cstr(), "s") == 0);
                CHECK(sum.scale() == 121);
            }
        } /*TEST_CASE(add2)*/

        TEST_CASE("add3", "[quantity]") {
            quantity<units::second, int64_t> t1 = seconds(1);
            quantity<units::minute, int64_t> t2 = minutes(2);

            /* sum will take unit from lhs argument to + */
            auto sum = t1 + t2;

            static_assert(sum.basis_power<dim::time> == 1);
            static_assert(std::same_as<decltype(sum)::unit_type, units::second>);

            REQUIRE(sum.scale() == 121);
        } /*TEST_CASE(add3)*/

        TEST_CASE("add4", "[quantity]") {
            constexpr bool c_debug_flag = false;

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.add4"));

            using u_kgps_result = unit_cartesian_product<units::kilogram, unit_invert_t<units::second>>;
            using u_kgps = u_kgps_result::exact_unit_type;
            using u_gpm_result = unit_cartesian_product<units::gram, unit_invert_t<units::minute>>;
            using u_gpm = u_gpm_result::exact_unit_type;
            {
                static_assert(u_kgps_result::c_scalefactor_inexact == 1.0);

                static_assert(std::same_as<unit_find_bpu_t<u_kgps, dim::mass>::power_type, std::ratio<1>>);
                static_assert(std::same_as<unit_find_bpu_t<u_kgps, dim::time>::power_type, std::ratio<-1>>);
                static_assert(std::same_as<unit_find_bpu_t<u_gpm, dim::mass>::power_type, std::ratio<1>>);
                static_assert(std::same_as<unit_find_bpu_t<u_gpm, dim::time>::power_type, std::ratio<-1>>);

                log && log(xtag("u_kgps", unit_abbrev_v<u_kgps>.c_str()));
                log && log(xtag("u_gpm", unit_abbrev_v<u_gpm>.c_str()));

                CHECK(strcmp(unit_abbrev_v<u_kgps>.c_str(), "kg.s^-1") == 0);
                CHECK(strcmp(unit_abbrev_v<u_gpm>.c_str(), "g.min^-1") == 0);

                static_assert(same_dimension_v<u_kgps, u_gpm>);
            }

            using convert_type = unit_conversion_factor_t<u_kgps, u_gpm>;
            {
                log && log(xtag("u_kgps->u_gpm", cstr_from_ratio<convert_type>()));

                CHECK(strcmp(cstr_from_ratio<convert_type>(), "60000") == 0);
                CHECK(from_ratio<int64_t, convert_type>() == 60000);
            }

            /* note: in practice probably write
             *   kilograms(0.1) / seconds(1);
             * but
             * 1. don't want to exercise quantity {*,/} here;
             * 2. want to force unit representation
             */
            auto q1 = quantity<u_kgps, double>::promote(0.1);
            auto q2 = quantity<u_gpm, double>();
            {
                q2 = q1;

                static_assert(q1.basis_power<dim::mass> == 1);
                static_assert(q1.basis_power<dim::time> == -1);
                static_assert(q2.basis_power<dim::mass> == 1);
                static_assert(q2.basis_power<dim::time> == -1);

                log && log(XTAG(q1), XTAG(q2));

                CHECK(strcmp(q1.unit_cstr(), "kg.s^-1") == 0);
                CHECK(q1.scale() == 0.1);

                CHECK(strcmp(q2.unit_cstr(), "g.min^-1") == 0);
                CHECK(q2.scale() == 6000.0);
            }
        } /*TEST_CASE(add4)*/

        TEST_CASE("add5", "[quantity][fractional_dimension]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.add5"));
            //log && log("(A)", xtag("foo", foo));

            auto vol_250d = volatility250d(0.2);
            {
                log && log(xtag("vol_250d", vol_250d));

                CHECK(strcmp(vol_250d.unit_cstr(), "yr250^-(1/2)") == 0);
                CHECK(vol_250d.scale() == 0.2);
            }

            /* scaling factor from 30-day vol to 250-day vol is sqrt(250/30) ~ 2.88675
             * so 0.1 -> 0.288675
             */
            auto vol_30d = volatility30d(0.1);
            {
                log && log(xtag("vol_30d", vol_30d));

                CHECK(strcmp(vol_30d.unit_cstr(), "mo^-(1/2)") == 0);
                CHECK(vol_30d.scale() == Approx(0.1).epsilon(1e-6));
            }

            /* conversion from monthly vol to (250-day) annual vol */

            using u_vol250d = units::volatility_250d;
            {
                quantity<u_vol250d, double> q = vol_30d;

                log && log(xtag("q", q));

                CHECK(strcmp(q.unit_cstr(), "yr250^-(1/2)") == 0);
                CHECK(q.scale() == Approx(0.288675).epsilon(1e-6));

            }

            {
                auto sum = vol_250d + vol_30d;

                static_assert(sum.basis_power<dim::time, double> == -0.5);

                log && log(XTAG(sum));

                CHECK(strcmp(sum.unit_cstr(), "yr250^-(1/2)") == 0);
                /* 0.1mo^-(1/2) ~ 0.288675yr250^-(1/2) */
                CHECK(sum.scale() == Approx(0.4886751).epsilon(1e-6));
            }
        } /*TEST_CASE(add5)*/


        TEST_CASE("mult1", "[quantity]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.mult1"));
            //log && log("(A)", xtag("foo", foo));

            auto q0 = milliseconds(5);
            auto q1 = seconds(60);
            auto q2 = minutes(1);

            {
                auto r = q0 * q1;

                static_assert(r.basis_power<dim::time> == 2);

                log && log(xtag("q0", q0), xtag("q1", q1), xtag("q0*q1", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* taking unit from LHS */
                REQUIRE(strcmp(r.unit_cstr(), "ms^2") == 0);
                REQUIRE(r.scale() == 300000);
            }

            {
                auto r = q1 * q2;

                static_assert(r.basis_power<dim::time> == 2);

                log && log(xtag("q1", q1), xtag("q2", q2), xtag("q1*q2", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* taking unit from LHS */
                REQUIRE(strcmp(r.unit_cstr(), "s^2") == 0);
                REQUIRE(r.scale() == 3600);
            }

            {
                auto r = q2 * q1;

                static_assert(r.basis_power<dim::time> == 2);

                log && log(xtag("q1", q1), xtag("q2", q2), xtag("r=q2*q1", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* taking unit from LHS */
                CHECK(strcmp(r.unit_cstr(), "min^2") == 0);
                CHECK(r.scale() == 1);
            }

            {
                auto r = q2 * 60;

                static_assert(r.basis_power<dim::time> == 1);
                static_assert(std::same_as<decltype(r)::repr_type, int>);

                log && log(xtag("q2*60", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* preserve units of existing quantity */
                CHECK(strcmp(r.unit_cstr(), "min") == 0);
                CHECK(r.scale() == 60);
            }

            {
                auto r = q2 * 60U;

                static_assert(r.basis_power<dim::time> == 1);
                static_assert(std::same_as<decltype(r)::repr_type, uint32_t>);

                log && log(xtag("q2*60U", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* preserve units of existing quantity */
                CHECK(strcmp(r.unit_cstr(), "min") == 0);
                CHECK(r.scale() == 60U);
            }

            {
                auto r = (q2 * 60.5);

                static_assert(r.basis_power<dim::time> == 1);

                /* verify dimension */
                static_assert(std::same_as<decltype(r)::repr_type, double>);

                log && log(xtag("q2*60.5", q2*60.5));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* preserve units of existing quantity */
                REQUIRE(strcmp(r.unit_cstr(), "min") == 0);
                REQUIRE(r.scale() == 60.5);
            }

            {
                log && log(xtag("q2*60.5f", q2*60.5f));

                auto r = (q2 * 60.5f);

                /* verify dimension */
                static_assert(r.basis_power<dim::time> == 1);
                static_assert(std::same_as<decltype(r)::repr_type, float>);

                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* preserve units of existing quantity */
                REQUIRE(strcmp(r.unit_cstr(), "min") == 0);
                REQUIRE(r.scale() == 60.5f);
            }

            {
                auto r = 60 * q2;

                static_assert(r.basis_power<dim::time> == 1);
                static_assert(std::same_as<decltype(r)::repr_type, int>);

                log && log(xtag("60*q2", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* preserve units of existing quantity */
                CHECK(strcmp(r.unit_cstr(), "min") == 0);
                CHECK(r.scale() == 60);
            }

            {
                log && log(xtag("60.5*q2", 60.5*q2));

                auto r = 60.5 * q2;

                static_assert(r.basis_power<dim::time> == 1);

                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));
                static_assert(std::same_as<decltype(r)::repr_type, double>);

                /* preserve units of existing quantity */
                CHECK(strcmp(r.unit_cstr(), "min") == 0);
                CHECK(r.scale() == 60.5);
            }

            {
                log && log(xtag("60.5f*q2", 60.5f*q2));

                auto r = 60.5f * q2;

                static_assert(r.basis_power<dim::time> == 1);
                static_assert(std::same_as<decltype(r)::repr_type, float>);

                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* preserve units of existing quantity */
                CHECK(strcmp(r.unit_cstr(), "min") == 0);
                CHECK(r.scale() == 60.5);
            }
        } /*TEST_CASE(mult1)*/

        TEST_CASE("div1", "[quantity]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.div1"));
            //log && log("(A)", xtag("foo", foo));

            auto q0 = milliseconds(5);
            auto q1 = milliseconds(10);

            {
                /* repr_type adopts argument to milliseconds() */
                static_assert(std::same_as<decltype(q0)::repr_type, int>);
                static_assert(std::same_as<decltype(q1)::repr_type, int>);

                auto r = q0/q1;

                log && log(xtag("q0", q0), xtag("q1", q1), xtag("q0/q1", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimensionless + no type promotion */
                static_assert(std::same_as<decltype(r), int>);
                /* verify scale (truncate)*/
                REQUIRE(r == 0);
            }

            auto q0p = milliseconds(5.0);

            {
                static_assert(std::same_as<decltype(q0p)::repr_type, double>);

                auto r = q0p/q1;
                static_assert(std::same_as<decltype(r), double>);

                log && log(XTAG(q0p), xtag("q0p/q1", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimension */
                static_assert(std::same_as<decltype(r), double>);

                /* verify scale */
                REQUIRE(r == 0.5);
            }

            auto r1 = 1.0 / q0;

            {
                log && log(XTAG(q0), xtag("r1=1.0/q0", r1));

                /* verify dimension */
                static_assert(r1.basis_power<dim::time> == -1);

                /* verify scale */
                REQUIRE(r1.scale() == 0.2);
            }
        } /*TEST_CASE(div1)*/

        TEST_CASE("div2", "[quantity]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.div2"));

            auto q0 = milliseconds(5);
            auto q1 = milliseconds(20.0);

            {
                auto r = q0/q1;

                log && log(xtag("q0", q0), xtag("q1", q1), xtag("q0/q1", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimension */
                static_assert(std::same_as<decltype(r), double>);

                /* verify scale */
                REQUIRE(r == 0.25);
            }

            {
                auto r = q1/q0;

                log && log(xtag("q0", q0), xtag("q1", q1), xtag("q1/q0", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimension */
                static_assert(std::same_as<decltype(r), double>);

                /* verify scale */
                REQUIRE(r == 4.0);
            }

            {
                auto r = q0/(q1*q1);

                log && log(xtag("q0", q0), xtag("q1", q1), xtag("q0/(q1*q1)", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimension */
                static_assert(r.basis_power<dim::time> == -1);

                /* verify scale */
                REQUIRE(r.scale() == 0.0125);
            }

            {
                auto r = (q0*q0)/q1;

                log && log(xtag("q0", q0), xtag("q1", q1), xtag("(q0*q0)/q1", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimension */
                static_assert(r.basis_power<dim::time> == 1);

                /* verify scale */
                REQUIRE(r.scale() == 1.25);
            }

        } /*TEST_CASE(div2)*/

        TEST_CASE("div3", "[quantity]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.div3"));

            auto q0 = milliseconds(5);
            auto q1 = milliseconds(20.0);

            {
                auto r = q0/q1;

                log && log(XTAG(q0), XTAG(q1), xtag("q0/q1", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimension */
                static_assert(std::same_as<decltype(r), double>);

                /* verify scale */
                REQUIRE(r == 0.25);
            }

            {
                auto r = q1/q0;

                log && log(xtag("q0", q0), xtag("q1", q1), xtag("q1/q0", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimension */
                static_assert(std::same_as<decltype(r), double>);

                /* verify scale */
                REQUIRE(r == 4.0);
            }

            {
                auto r = q0/(q1*q1);

                log && log(xtag("q0", q0), xtag("q1", q1), xtag("q0/(q1*q1)", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimension */
                static_assert(r.basis_power<dim::time> == -1);

                /* verify scale */
                REQUIRE(r.scale() == 0.0125);
            }

            {
                auto r = (q0*q0)/q1;

                log && log(xtag("q0", q0), xtag("q1", q1), xtag("(q0*q0)/q1", r));
                log && log(xtag("r.type", Reflect::require<decltype(r)>()->canonical_name()));

                /* verify dimension */
                static_assert(r.basis_power<dim::time> == 1);

                /* verify scale */
                REQUIRE(r.scale() == 1.25);
            }

        } /*TEST_CASE(div3)*/

        TEST_CASE("div4", "[quantity]") {
            /* test with exact scalefactor */

            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.div4"));
            //log && log("(A)", xtag("foo", foo));

            auto q1 = milliseconds(1);
            auto q2 = minutes(1);

            auto r = q1 / q2.with_repr<double>();

            /* 0.1/sqrt(30dy) ~ 0.288675/sqrt(250dy),
             * so q1/q2 ~ 0.6928
             */

            log && log(XTAG(q1), XTAG(q2), xtag("q1/q2", r));

            /* verify dimensionless result */
            static_assert(std::same_as<decltype(r), double>);

            /* verify scale of result */
            CHECK(r == Approx(0.00001666667).epsilon(1e-6));

        } /*TEST_CASE(div4)*/

        TEST_CASE("div5", "[quantity]") {
            /* test with inexact scalefactor */

            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.div5"));
            //log && log("(A)", xtag("foo", foo));

            auto q1 = volatility250d(0.2);
            auto q2 = volatility30d(0.1);

            auto r = q1/q2;

            /* 0.1/sqrt(30dy) ~ 0.288675/sqrt(250dy),
             * so q1/q2 ~ 0.6928
             */

            log && log(XTAG(q1), XTAG(q2), XTAG(q1/q2));

            /* verify dimensionless result */
            static_assert(std::same_as<decltype(r), double>);

            /* verify scale of result */
            CHECK(r == Approx(0.692820323).epsilon(1e-6));

        } /*TEST_CASE(div5)*/

        TEST_CASE("muldiv5", "[quantity]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.muldiv5"));
            //log && log("(A)", xtag("foo", foo));

            auto t = milliseconds(10);
            auto m = kilograms(2.5);

            auto a = m / (t * t);

            /* 0.1/sqrt(30dy) ~ 0.288675/sqrt(250dy),
             * so q1/q2 ~ 0.6928
             */

            log && log(XTAG(m), XTAG(t), xtag("a=m.t^-2", a));

            /* verify dimensions of result + sticky units */
            CHECK(strcmp(t.unit_cstr(), "ms") == 0);
            CHECK(strcmp(m.unit_cstr(), "kg") == 0);
            CHECK(strcmp(a.unit_cstr(), "kg.ms^-2") == 0);

            CHECK(a.scale() == 0.025);

            /* verify scale of result */
            //CHECK(r == Approx(0.692820323).epsilon(1e-6));

        } /*TEST_CASE(muldiv5)*/

        TEST_CASE("rescale", "[quantity]") {
            constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.rescale"));
            //log && log("(A)", xtag("foo", foo));

            auto q = kilograms(150.0) / minutes(1); /* 150.0kg.min^-1 */

            CHECK(strcmp(q.unit_cstr(), "kg.min^-1") == 0);
            CHECK(q.scale() == 150.0);

            log && log(XTAG(q));

            namespace u = xo::unit::units;

            auto q1 = q.with_basis_unit<u::millisecond>(); /* 0.0025kg.ms^-1 */

            CHECK(strcmp(q1.unit_cstr(), "kg.ms^-1") == 0);
            CHECK(q1.scale() == 0.0025);

            log && log(XTAG(q1));

            auto q2 = q1.with_basis_unit<u::gram>(); /* 2.5g.ms^-1 */

            CHECK(strcmp(q2.unit_cstr(), "g.ms^-1") == 0);
            CHECK(q2.scale() == 2.5);

            log && log(XTAG(q2));

            auto q3 = q2.with_basis_unit<u::second>(); /* 2500g.s^-1 */

            CHECK(strcmp(q3.unit_cstr(), "g.s^-1") == 0);
            CHECK(q3.scale() == 2500.0);

            log && log(XTAG(q3));
        } /*TEST_CASE(rescale)*/

        TEST_CASE("rescale2", "[quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.rescale2"));
            //log && log("(A)", xtag("foo", foo));

            namespace u = xo::unit::unit_qty;

            auto q1 = kilometers(150.0) / u::hour;
            auto q2 = q1.with_units_from(u::meter / u::second);

            log && log(XTAG(q1), XTAG(q2));
        } /*TEST_CASE(rescale2)*/

        TEST_CASE("compare1", "[quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.compare1"));
            //log && log("(A)", xtag("foo", foo));

            namespace u = xo::unit::unit_qty;

            auto q1 = kilometers(150.0) / u::hour;
            auto q2 = kilometers(100.0) / u::hour;

            CHECK(is_gt(q1 <=> q2));
            CHECK(is_eq(q1 <=> q1));
            CHECK(is_lt(q2 <=> q1));
            CHECK(q1 == q1);
            CHECK(q1 != q2);
            CHECK(q1 >= q1);
            CHECK(q1 <= q1);

            CHECK(q1 > q2);
            CHECK(q1 >= q2);

            CHECK(q2 < q1);
            CHECK(q2 <= q1);

            log && log(XTAG(q1), XTAG(q2), XTAG(is_gt(q1<=>q2)));
        } /*TEST_CASE(compare1)*/

        TEST_CASE("compare2", "[quantity]") {
            constexpr bool c_debug_flag = true;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.compare2"));
            //log && log("(A)", xtag("foo", foo));

            namespace u = xo::unit::unit_qty;

            auto q1 = kilometers(150.0) / u::hour;
            auto q2 = meters(30.0) / u::second;

            CHECK(is_gt(q1 <=> q2));
            CHECK(is_eq(q1 <=> q1));
            CHECK(is_lt(q2 <=> q1));
            CHECK(q1 == q1);
            CHECK(q1 != q2);
            CHECK(q1 >= q1);
            CHECK(q1 <= q1);

            CHECK(q1 > q2);
            CHECK(q1 >= q2);

            CHECK(q2 < q1);
            CHECK(q2 <= q1);

            log && log(XTAG(q1), XTAG(q2), XTAG(is_gt(q1<=>q2)));
        } /*TEST_CASE(compare2)*/

    } /*namespace ut*/
} /*namespace xo*/

/* end quantity.test.cpp */
