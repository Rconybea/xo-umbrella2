/** @file DArenaHashMap.test.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArenaHashMap.hpp"
#include "random_hash_ops.hpp"
#include <xo/randomgen/random_seed.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <xo/indentlog/print/hex.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::map::DArenaHashMapUtil;
    using xo::map::DArenaHashMap;
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

                REQUIRE(map.verify_ok(verify_policy::chatty()));
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

                REQUIRE(map.verify_ok(verify_policy::chatty()));
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

                REQUIRE(map.verify_ok(verify_policy::chatty()));
            }

            {
                map.clear();

                REQUIRE(map.empty());
                REQUIRE(map.size() == 0);
                REQUIRE(map.groups() == 0);
                REQUIRE(map.capacity() == 0);

                REQUIRE(map.verify_ok(verify_policy::chatty()));
            }

            /* slightly different starting point, 0 capacity! */
            {
                auto x = map.try_insert(std::make_pair(1, 11));

                /* try_insert should fail - no capacity */
                REQUIRE(!x.first);
                REQUIRE(!x.second);

                REQUIRE(map.verify_ok(verify_policy::chatty()));
            }

            {
                /* insert will grow hash table */
                auto x = map.insert(std::make_pair(1, 11));

                CHECK(x);
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

                REQUIRE(map.verify_ok(verify_policy::chatty()));
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

            for (std::uint32_t n = 0; n <= 8; ) {
                HashMap hash_map;

                auto test_fn = [&rgen, &hash_map](bool dbg_flag,
                                                  std::uint32_t n)
                    {
                        bool ok_flag = true;

                        ok_flag &= HashMapUtil<HashMap>::random_inserts(n, dbg_flag, &rgen, &hash_map);

                        ok_flag &= HashMapUtil<HashMap>::check_forward_iterator(0.0 /*dvalue*/,
                                                                                dbg_flag, hash_map);
                        /* regular forward iterator, but start at hash_map.end() and use operator-- */
                        ok_flag &= HashMapUtil<HashMap>::check_backward_iterator(0.0 /*dvalue*/,
                                                                                 dbg_flag, hash_map);

                        ok_flag &= HashMapUtil<HashMap>::random_lookups(0.0 /*dvalue*/,
                                                                        dbg_flag, &rgen, hash_map);

                        return ok_flag;
                    };

                bool ok_flag = UtestTools::bimodal_test("DArenaHashMap-try-insert2", test_fn, n);

                if (n == 0)
                    n = 1;
                else
                    n = 2*n;
            }
        }

        TEST_CASE("DArenaHashMap-operator-bracket", "[arena][DArenaHashMap]")
        {
            scope log(XO_DEBUG(false));

            using HashMap = DArenaHashMap<int, int>;

            HashMap map;

            // copy keys here so we can print stuff
            std::vector<int> key_v;

            // insert via operator[]
            map[1] = 100;
            key_v.push_back(1);
            REQUIRE(map.verify_ok(verify_policy::chatty()));

            map[2] = 200;
            key_v.push_back(2);
            REQUIRE(map.verify_ok(verify_policy::chatty()));

            map[3] = 300;
            key_v.push_back(3);
            REQUIRE(map.verify_ok(verify_policy::chatty()));

            REQUIRE(map.size() == 3);

            // read back via operator[]
            REQUIRE(map[1] == 100);
            REQUIRE(map[2] == 200);
            REQUIRE(map[3] == 300);

            // update via operator[]
            map[2] = 250;
            REQUIRE(map[2] == 250);
            REQUIRE(map.size() == 3);  // size unchanged
            REQUIRE(map.verify_ok(verify_policy::chatty()));

            // verify via find
            {
                auto it = map.find(1);
                REQUIRE(it != map.end());
                REQUIRE(it->second == 100);
            }
            {
                auto it = map.find(2);
                REQUIRE(it != map.end());
                REQUIRE(it->second == 250);
            }
            {
                auto it = map.find(3);
                REQUIRE(it != map.end());
                REQUIRE(it->second == 300);
            }
            {
                auto it = map.find(4);
                REQUIRE(it == map.end());
            }

            REQUIRE(map.verify_ok(verify_policy::chatty()));

            // operator[] on non-existent key creates default entry
            int & val = map[999];
            key_v.push_back(999);

            for (uint64_t i_slot = 0, N = map._store()->n_slot_; i_slot < N; ++i_slot) {
                auto key = map._store()->slots_[i_slot].first;
                auto ctrl = map._store()->control_
                    [i_slot + DArenaHashMapUtil::c_control_stub];
                auto isdata = DArenaHashMapUtil::is_data(ctrl);
                auto [h1,h2] = map._hash(key);

                if ((key != 0)
                    || (h1 != 0)
                    || (h2 != 0)
                    || (ctrl != DArenaHashMapUtil::c_empty_slot)
                    || isdata
                    ) {
                log && log(xtag("i", i_slot),
                           xtag("key[i]", key),
                           xtag("h1", h1), xtag("h2", h2),
                           xtag("ctrl[i]", (int)ctrl),
                           xtag("isdata", isdata));
                }
            }

            REQUIRE(map.verify_ok(verify_policy::chatty()));

            REQUIRE(map.size() == 4);
            REQUIRE(val == 0);  // default-initialized
            val = 999;
            REQUIRE(map[999] == 999);
        }

        TEST_CASE("DArenaHashMap-string_view-key", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<std::string_view, int>;

            HashMap map(1024);

            REQUIRE(map.verify_ok());

            map["hello"] = 42;
            REQUIRE(map.size() == 1);
            REQUIRE(map.verify_ok());

            map["world"] = 100;
            REQUIRE(map.size() == 2);
            REQUIRE(map.verify_ok());

            REQUIRE(map["hello"] == 42);
            REQUIRE(map["world"] == 100);
        }

        // TODO:
        //  - let's try getting lcov to work in xo-umbrella2
    }
}

/* end DArenaHashMap.test.cpp */
