/* @file bpu.test.cpp */

#include "xo/unit/bpu.hpp"
//#include "xo/indentlog/scope.hpp"
#include <catch2/catch.hpp>

namespace xo {
    namespace qty {
        using bpu64_type = bpu<std::int64_t>;

        /* compile-time test:
         * verify we can use a bpu64_type instance as a non-type template parameter.
         * Will need this for quantity<Repr, Int, natural_unit<Int>>
         */
        template <bpu64_type bpu>
        constexpr bpu_abbrev_type bpu_mpl_abbrev = bpu.abbrev();

        TEST_CASE("bpu", "[bpu]") {
            //constexpr bool c_debug_flag = false;

            // can get bits from /dev/random by uncommenting the 2nd line below
            //uint64_t seed = xxx;
            //rng::Seed<xoshio256ss> seed;

            //auto rng = xo::rng::xoshiro256ss(seed);

            //scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.bpu"));
            //log && log("(A)", xtag("foo", foo));

            static_assert(bpu_mpl_abbrev<bpu64_type::unit_power(bu::gram)> == bpu64_type::unit_power(bu::gram).abbrev());
            REQUIRE(bpu_mpl_abbrev<bpu64_type::unit_power(bu::gram)> == bpu64_type::unit_power(bu::gram).abbrev());
        } /*TEST_CASE(bpu)*/
    } /*namespace qty*/
} /*namespace xo*/

/* end bpu.test.cpp */
