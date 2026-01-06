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

        TEST_CASE("DArenaVector-resize-expand", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            REQUIRE(vec.size() == 0);

            // resize from 0 to 5
            vec.resize(5);
            REQUIRE(vec.size() == 5);

            // can write to all indices
            for (size_t i = 0; i < 5; ++i) {
                vec[i] = static_cast<double>(i * 10);
            }

            REQUIRE(vec[0] == 0.0);
            REQUIRE(vec[1] == 10.0);
            REQUIRE(vec[2] == 20.0);
            REQUIRE(vec[3] == 30.0);
            REQUIRE(vec[4] == 40.0);

            // resize to larger
            vec.resize(8);
            REQUIRE(vec.size() == 8);

            // original values preserved
            REQUIRE(vec[0] == 0.0);
            REQUIRE(vec[1] == 10.0);
            REQUIRE(vec[4] == 40.0);
        }

        TEST_CASE("DArenaVector-resize-shrink", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            vec.push_back(1.0);
            vec.push_back(2.0);
            vec.push_back(3.0);
            vec.push_back(4.0);
            vec.push_back(5.0);

            REQUIRE(vec.size() == 5);

            // shrink to 3
            vec.resize(3);
            REQUIRE(vec.size() == 3);

            // first 3 elements preserved
            REQUIRE(vec[0] == 1.0);
            REQUIRE(vec[1] == 2.0);
            REQUIRE(vec[2] == 3.0);

            // index 3 now out of bounds
            REQUIRE_THROWS_AS(vec.at(3), std::out_of_range);

            // shrink to 0
            vec.resize(0);
            REQUIRE(vec.size() == 0);
            REQUIRE(vec.empty());
        }

        TEST_CASE("DArenaVector-resize-same", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            vec.push_back(10.0);
            vec.push_back(20.0);
            vec.push_back(30.0);

            REQUIRE(vec.size() == 3);

            // resize to same size
            vec.resize(3);
            REQUIRE(vec.size() == 3);

            // values unchanged
            REQUIRE(vec[0] == 10.0);
            REQUIRE(vec[1] == 20.0);
            REQUIRE(vec[2] == 30.0);
        }

        TEST_CASE("DArenaVector-clear", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            vec.push_back(1.0);
            vec.push_back(2.0);
            vec.push_back(3.0);

            REQUIRE(vec.size() == 3);
            REQUIRE(!vec.empty());

            vec.clear();

            REQUIRE(vec.size() == 0);
            REQUIRE(vec.empty());

            // can still push after clear
            vec.push_back(99.0);
            REQUIRE(vec.size() == 1);
            REQUIRE(vec[0] == 99.0);
        }

        TEST_CASE("DArenaVector-iterators", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            vec.push_back(10.0);
            vec.push_back(20.0);
            vec.push_back(30.0);

            // begin/end
            REQUIRE(vec.begin() != vec.end());
            REQUIRE(vec.end() - vec.begin() == 3);

            // iterate with pointer arithmetic
            auto it = vec.begin();
            REQUIRE(*it == 10.0);
            ++it;
            REQUIRE(*it == 20.0);
            ++it;
            REQUIRE(*it == 30.0);
            ++it;
            REQUIRE(it == vec.end());

            // modify through iterator
            *vec.begin() = 15.0;
            REQUIRE(vec[0] == 15.0);
        }

        TEST_CASE("DArenaVector-const-iterators", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            vec.push_back(1.0);
            vec.push_back(2.0);
            vec.push_back(3.0);

            const DArenaVector<double> & cvec = vec;

            REQUIRE(cvec.cbegin() != cvec.cend());
            REQUIRE(cvec.begin() == cvec.cbegin());
            REQUIRE(cvec.end() == cvec.cend());

            auto it = cvec.cbegin();
            REQUIRE(*it == 1.0);
            ++it;
            REQUIRE(*it == 2.0);
        }

        TEST_CASE("DArenaVector-range-for", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            vec.push_back(1.0);
            vec.push_back(2.0);
            vec.push_back(3.0);

            // read via range-for
            double sum = 0.0;
            for (double x : vec) {
                sum += x;
            }
            REQUIRE(sum == 6.0);

            // modify via range-for
            for (double & x : vec) {
                x *= 2.0;
            }
            REQUIRE(vec[0] == 2.0);
            REQUIRE(vec[1] == 4.0);
            REQUIRE(vec[2] == 6.0);
        }

        TEST_CASE("DArenaVector-reserve", "[arena][DArenaVector]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4096 };
            DArenaVector<double> vec = DArenaVector<double>::map(cfg);

            REQUIRE(vec.size() == 0);
            REQUIRE(vec.capacity() > 0);

            size_t initial_capacity = vec.capacity();

            // reserve doesn't change size
            vec.reserve(100);
            REQUIRE(vec.size() == 0);
            REQUIRE(vec.capacity() >= 100);

            // add some elements
            vec.push_back(1.0);
            vec.push_back(2.0);
            vec.push_back(3.0);

            REQUIRE(vec.size() == 3);
            size_t cap_after_push = vec.capacity();

            // reserve more space
            vec.reserve(200);
            REQUIRE(vec.size() == 3);
            REQUIRE(vec.capacity() >= 200);

            // values still intact
            REQUIRE(vec[0] == 1.0);
            REQUIRE(vec[1] == 2.0);
            REQUIRE(vec[2] == 3.0);
        }

        TEST_CASE("DArenaVector-swap", "[arena][DArenaVector]")
        {
            ArenaConfig cfg1 { .name_ = "testarena1",
                               .size_ = 4096 };
            ArenaConfig cfg2 { .name_ = "testarena2",
                               .size_ = 4096 };

            DArenaVector<double> vec1 = DArenaVector<double>::map(cfg1);
            DArenaVector<double> vec2 = DArenaVector<double>::map(cfg2);

            vec1.push_back(1.0);
            vec1.push_back(2.0);

            vec2.push_back(10.0);
            vec2.push_back(20.0);
            vec2.push_back(30.0);

            REQUIRE(vec1.size() == 2);
            REQUIRE(vec2.size() == 3);

            vec1.swap(vec2);

            // sizes swapped
            REQUIRE(vec1.size() == 3);
            REQUIRE(vec2.size() == 2);

            // contents swapped
            REQUIRE(vec1[0] == 10.0);
            REQUIRE(vec1[1] == 20.0);
            REQUIRE(vec1[2] == 30.0);

            REQUIRE(vec2[0] == 1.0);
            REQUIRE(vec2[1] == 2.0);
        }
    }
}

/* end DArenaVector.test.cpp */
