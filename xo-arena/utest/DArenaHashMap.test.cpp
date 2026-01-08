/** @file DArenaHashMap.test.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArenaHashMap.hpp"
#include "random_hash_ops.hpp"
#include <xo/randomgen/random_seed.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::DArenaHashMapUtil;
    using xo::mm::DArenaHashMap;
    using xo::rng::random_seed;
    using xo::rng::xoshiro256ss;
    using utest::UtestTools;
    using utest::HashMapUtil;

    namespace ut {
        TEST_CASE("DArenaHashMap-ctor", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            HashMap map;

            REQUIRE(map.empty());
            REQUIRE(map.size() == 0);
            REQUIRE(map.groups() == 1);
            REQUIRE(map.capacity() == DArenaHashMapUtil::c_group_size);
        }

        TEST_CASE("DArenaHashMap-ctor2", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            HashMap map(257);

            REQUIRE(map.empty());
            REQUIRE(map.size() == 0);
            REQUIRE(map.capacity() == map.groups() * DArenaHashMapUtil::c_group_size);
            REQUIRE(map.capacity() == std::max(512ul,
                                               DArenaHashMapUtil::c_group_size));
        }

        TEST_CASE("DArenaHashMap-try-insert", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            HashMap map;

            REQUIRE(map.empty());
            REQUIRE(map.size() == 0);
            REQUIRE(map.groups() == 1);
            REQUIRE(map.capacity() == DArenaHashMapUtil::c_group_size);

            {
                auto x = map.try_insert(std::make_pair(1, 11));

                REQUIRE(x.first);
                REQUIRE(x.second);
                REQUIRE(!map.empty());
                REQUIRE(map.size() == 1);
                REQUIRE(map.groups() == 1);
                REQUIRE(map.capacity() == DArenaHashMapUtil::c_group_size);
                REQUIRE(map.load_factor() == 1/16.0);

                /* verify iteration */
                {
                    size_t n = 0;
                    for (auto & ix : map) {
                        REQUIRE(ix.first == 1);
                        REQUIRE(ix.second == 11);
                        ++n;
                    }
                    REQUIRE(n == map.size());
                }
            }

            {
                auto x = map.try_insert(std::make_pair(2, 9));

                REQUIRE(x.first);
                REQUIRE(x.second);
                REQUIRE(!map.empty());
                REQUIRE(map.size() == 2);
                REQUIRE(map.groups() == 1);
                REQUIRE(map.capacity() == DArenaHashMapUtil::c_group_size);
                REQUIRE(map.load_factor() == 2/16.0);

                /* verify iteration */
                {
                    size_t n = 0;
                    for (auto & ix : map) {
                        ++n;
                    }
                    REQUIRE(n == map.size());
                }
            }

            {
                auto x = map.try_insert(std::make_pair(259, 12));

                REQUIRE(x.first);
                REQUIRE(x.second);
                REQUIRE(!map.empty());
                REQUIRE(map.size() == 3);
                REQUIRE(map.groups() == 1);
                REQUIRE(map.capacity() == DArenaHashMapUtil::c_group_size);
                REQUIRE(map.load_factor() == 3/16.0);

                /* verify iteration */
                {
                    size_t n = 0;
                    for (auto & ix : map) {
                        switch (ix.first) {
                        case 1:
                            REQUIRE(ix.second == 11);
                            break;
                        case 2:
                            REQUIRE(ix.second == 9);
                            break;
                        case 259:
                            REQUIRE(ix.second == 12);
                            break;
                        default:
                            REQUIRE(false);
                        }
                        ++n;
                    }
                    REQUIRE(n == map.size());
                }
            }
        }

        TEST_CASE("DArenaHashMap-try-insert2", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            std::uint64_t seed = 17747889312058974961ul;
            //random_seed(&seed); // to get new random seed
            //log && log(xtag("seed", seed));

            auto rgen = xoshiro256ss(seed);

            /* 1. Perform series of tests with increasing scale
             * 2. Each test may run in two modes:
             *    a. silent fast fail. just report success.
             *       In this mode avoid catch2 REQUIRE
             *    b. noisy. run with logging enabled
             *       This mode automatically invoked when silent mode
             *       observes test failure
             */

            for (std::uint32_t n = 0; n <= 2; ) {
                HashMap hash_map;

                auto test_fn = [&rgen, &hash_map](bool dbg_flag,
                                                  std::uint32_t n)
                    {
                        bool ok_flag = true;

                        ok_flag &= HashMapUtil<HashMap>::random_inserts(n, dbg_flag, &rgen, &hash_map);

                        return ok_flag;
                    };

                bool ok_flag = UtestTools::bimodal_test("DArenaHashMap-try-insert2", test_fn, n);

                if (n == 0)
                    n = 1;
                else
                    n = 2*n;
            }
        }

        // TODO:
        //  - let's try getting lcov to work in xo-umbrella2
    }
}

/* end DArenaHashMap.test.cpp */
