/** @file Generation.test.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include <xo/testutil/Utest.hpp>
#include "Generation.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::Generation;
    using xo::mm::c_max_generation;

    namespace ut {

        TEST_CASE("Generation-1", "[Generation]")
        {
            auto log = Utest::ut_scope();

            REQUIRE(Generation::nursery() == 0);
            REQUIRE(Generation::g0() == 0);
            REQUIRE(Generation::g1() == 1);
            REQUIRE(Generation::sentinel() == c_max_generation);

            REQUIRE(Generation::g0().is_sentinel() == false);
            REQUIRE(Generation::g1().is_sentinel() == false);
            REQUIRE(Generation::sentinel().is_sentinel());

            REQUIRE(Generation::g0() != Generation::sentinel());
            REQUIRE(Generation::g1() != Generation::sentinel());

            REQUIRE(!(Generation::g0() > Generation::g1()));
            REQUIRE(Generation::g1() > Generation::g0());

            auto g = Generation::g0();
            ++g;
            REQUIRE(g == Generation::g1());
            ++g;
            REQUIRE(g > Generation::g1());
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end Generation.test.cpp */
