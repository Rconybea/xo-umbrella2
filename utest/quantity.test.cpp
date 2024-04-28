/* @file quantity.test.cpp */

#include "xo/unit/quantity.hpp"
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

            constexpr auto g = qty::grams(1.0);

            static_assert(g.scale() == 1.0);

            log && log(xtag("g.abbrev", g.abbrev()));
        } /*TEST_CASE(quantity)*/
    } /*namespace qty*/
} /*namespace xo*/

/* end quantity.test.cpp */
