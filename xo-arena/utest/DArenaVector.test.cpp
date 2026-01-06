/** @file DArenaVector.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "xo/arena/DArenaVector.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::DArenaVector;
    using xo::mm::ArenaConfig;
    using std::byte;

    namespace ut {
        TEST_CASE("DArenaVector-tiny", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 1 };
            DArenaVector<double> arenavec = DArenaVector<double>::map(cfg);

            REQUIRE(arenavec.empty());
        }

        TEST_CASE("DArenaVector-push_back-rvalue", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            REQUIRE(vec.empty());
            REQUIRE(vec.size() == 0);

            vec.push_back(1.5);

            REQUIRE(!vec.empty());
            REQUIRE(vec.size() == 1);
            REQUIRE(vec[0] == 1.5);

            vec.push_back(2.5);
            vec.push_back(3.5);

            REQUIRE(vec.size() == 3);
            REQUIRE(vec[0] == 1.5);
            REQUIRE(vec[1] == 2.5);
            REQUIRE(vec[2] == 3.5);
        }

        TEST_CASE("DArenaVector-push_back-lvalue", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            double a = 10.0;
            double b = 20.0;
            double c = 30.0;

            vec.push_back(a);

            REQUIRE(vec.size() == 1);
            REQUIRE(vec[0] == 10.0);

            vec.push_back(b);
            vec.push_back(c);

            REQUIRE(vec.size() == 3);
            REQUIRE(vec[0] == 10.0);
            REQUIRE(vec[1] == 20.0);
            REQUIRE(vec[2] == 30.0);
        }
    }
}

/* end DArenaVector.test.cpp */
