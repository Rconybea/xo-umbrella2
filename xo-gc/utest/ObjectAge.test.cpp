/** @file ObjectAge.test.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "object_age.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::object_age;

    namespace ut {

        TEST_CASE("ObjectAge-1", "[ObjectAge]")
        {
            REQUIRE(object_age{0} != object_age{1});
            REQUIRE(object_age{0} < object_age{1});
            REQUIRE(object_age{1} > object_age{0});

            {
                bool x = (object_age{0} > object_age{1});
                REQUIRE(x == false);
            }

            {
                bool x = (object_age{1} < object_age{0});
                REQUIRE(x == false);
            }
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end ObjectAge.test.cpp */
