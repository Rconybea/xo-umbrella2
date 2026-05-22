/** @file VisitReason.test.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include <xo/testutil/Utest.hpp>
#include <xo/alloc2/VisitReason.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::VisitReason;

    namespace ut {

        TEST_CASE("VisitReason-1", "[visitreason]")
        {
            auto log = Utest::ut_scope();

            REQUIRE(VisitReason::unspecified() == VisitReason::unspecified());

            REQUIRE(VisitReason::unspecified() != VisitReason::forward());
            REQUIRE(VisitReason::unspecified() != VisitReason::verify());
            REQUIRE(VisitReason::forward() != VisitReason::verify());
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end VisitReason.test.cpp */
