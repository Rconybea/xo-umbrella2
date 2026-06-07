/** @file Role.test.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include <xo/testutil/Utest.hpp>
#include "role.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::Role;

    namespace ut {

        TEST_CASE("Role-1", "[Role]")
        {
            auto log = Utest::ut_scope();

            /* 1. there are two distinct valid roles, 'to' and 'from',
             * 2. valid roles fall in interval [begin, end)
             */

            REQUIRE(Role::to_space() == Role::to_space());
            REQUIRE(Role::from_space() == Role::from_space());
            REQUIRE(Role::to_space() != Role::from_space());

            Role x0 = Role::begin();
            {
                bool ok = (x0 == Role::to_space() || x0 == Role::from_space());
                REQUIRE(ok);
            }
            REQUIRE(x0 != Role::end());

            Role x1 = x0.next();
            REQUIRE(x1 != x0);
            {
                bool ok = (x1 == Role::to_space() || x1 == Role::from_space());
                REQUIRE(ok);
            }
            REQUIRE(x1 != Role::end());

            Role x2 = x1.next();
            REQUIRE(x2 == Role::end());
        }

    }
} /*namespace xo*/

/* end Role.test.cpp */
