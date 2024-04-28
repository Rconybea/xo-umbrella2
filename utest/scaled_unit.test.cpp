/* @file scaled_unit.test.cpp */

#include "xo/unit/scaled_unit.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace qty {
        using su64_type = scaled_unit<std::int64_t>;

        /* compile-time test:
         * verify we can use an su64_type instance as a non-type template parameter.
         * Will need this for quantity<Repr, Int, scaled_unit<Int>>
         */
        template <su64_type su>
        constexpr su64_type su_reciprocal = su.reciprocal();

        TEST_CASE("scaled_unit", "[scaled_unit]") {
            //constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            //scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu"));
            //log && log("(A)", xtag("foo", foo));

            static_assert(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.natural_unit_ == nu::gram.reciprocal());
            REQUIRE(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.natural_unit_ == nu::gram.reciprocal());

            static_assert(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.outer_scale_factor_ == 1);
            REQUIRE(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.outer_scale_factor_ == 1);

            static_assert(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.outer_scale_sq_ == 1.0);
            REQUIRE(su_reciprocal<scaled_unit(nu::gram, xo::ratio::ratio(1L), 1)>.outer_scale_sq_ == 1.0);
        } /*TEST_CASE(scaled_unit)*/
    } /*namespace qty*/
} /*namespace xo*/

/* end scaled_unit.test.cpp */
