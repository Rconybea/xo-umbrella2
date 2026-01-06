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

        TEST_CASE("DArenaVector-at-valid", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            vec.push_back(100.0);
            vec.push_back(200.0);
            vec.push_back(300.0);

            REQUIRE(vec.at(0) == 100.0);
            REQUIRE(vec.at(1) == 200.0);
            REQUIRE(vec.at(2) == 300.0);

            // test mutability via at()
            vec.at(1) = 250.0;
            REQUIRE(vec.at(1) == 250.0);
        }

        TEST_CASE("DArenaVector-at-throws", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            // empty vector - any index is invalid
            REQUIRE_THROWS_AS(vec.at(0), std::out_of_range);

            vec.push_back(1.0);
            vec.push_back(2.0);

            // valid indices work
            REQUIRE_NOTHROW(vec.at(0));
            REQUIRE_NOTHROW(vec.at(1));

            // index == size is invalid
            REQUIRE_THROWS_AS(vec.at(2), std::out_of_range);

            // index > size is invalid
            REQUIRE_THROWS_AS(vec.at(100), std::out_of_range);
        }
    }
}

/* end DArenaVector.test.cpp */
