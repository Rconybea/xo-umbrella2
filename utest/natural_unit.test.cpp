/* @file natural_unit.test.cpp */

#include "xo/unit/natural_unit.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace qty {
        using nu64_type = natural_unit<std::int64_t>;

        /* compile-time test:
         * verify we can use an nu64_type instance as a non-type template parameter.
         * Will need this for quantity<Repr, Int, natural_unit<Int>>
         */
        template <nu64_type nu>
        constexpr nu_abbrev_type nu_mpl_abbrev = nu.abbrev();

        TEST_CASE("natural_unit", "[natural_unit]") {
            //constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            //scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu"));
            //log && log("(A)", xtag("foo", foo));

            static_assert(nu_mpl_abbrev<nu::gram> == nu::gram.abbrev());
            REQUIRE(nu_mpl_abbrev<nu::gram> == nu::gram.abbrev());

        } /*TEST_CASE(natural_unit)*/
    } /*namespace qty*/
} /*namespace xo*/


/* end natural_unit.test.cpp */
