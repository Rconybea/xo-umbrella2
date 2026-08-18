/** @file DArenaHashMap.test.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArenaHashMap.hpp"
#include "random_hash_ops.hpp"
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/testutil/try_test_array.hpp>
#include <xo/randomgen/random_seed.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::pp::scope;
    using xo::pp::xtag;
    using xo::map::DArenaHashMapUtil;
    using xo::map::DArenaHashMap;
    using xo::rng::random_seed;
    using xo::rng::xoshiro256ss;
    using xo::UtestRehearser;
    using xo::try_test_array;
    using utest::HashMapUtil;

    namespace ut {
        TEST_CASE("DArenaHashMap-ctor", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            HashMap map("utest");

            REQUIRE(map.empty());
            REQUIRE(map.size() == 0);
            REQUIRE(map.groups() == 1);
            REQUIRE(map.capacity() == DArenaHashMapUtil::c_group_size);
        }

        TEST_CASE("DArenaHashMap-ctor2", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            HashMap map("utest", 257);

            REQUIRE(map.empty());
            REQUIRE(map.size() == 0);
            REQUIRE(map.capacity() == map.groups() * DArenaHashMapUtil::c_group_size);
            REQUIRE(map.capacity() == std::max(512ul,
                                               DArenaHashMapUtil::c_group_size));
        }

        TEST_CASE("DArenaHashMap-try-insert", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            HashMap map("utest");

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

        /** Initial state for a hashmap test case **/
        struct TestCase_HashMap {
            /* number of keys to insert */
            std::uint32_t n_ = 0;
            /* seed for this case; a case must be reproducible on its own */
            std::uint64_t seed_ = 0;
        };

        /** vector of hashmap test cases from seed **/
        static std::vector<TestCase_HashMap>
        hashmap_scales(std::uint64_t seed)
        {
            std::vector<TestCase_HashMap> tc_v;

            for (std::uint32_t n = 0; n <= 256; n = (n == 0) ? 1 : 2*n)
                tc_v.push_back(TestCase_HashMap{.n_ = n, .seed_ = seed + n});

            return tc_v;
        }

        /* try_insert() does not grow the table.
         * It reports failure once load_factor() reaches c_max_load_factor.
         * Scale therefore needs a capacity hint.
         * Growth is covered by DArenaHashMap-grow below.
         */
        void
        hashmap_try_insert_test_fn(const TestCase_HashMap & tc,
                                   UtestRehearser * p_rh)
        {
            using HashMap = DArenaHashMap<int, int>;

            bool dbg_flag = p_rh->enable_debug();
            bool ok_flag = true;

            /* fresh for each pass */
            auto rgen = xoshiro256ss(tc.seed_);
            HashMap hash_map("utest", 2 * tc.n_ + 1);

            ok_flag &= HashMapUtil<HashMap>::random_inserts(tc.n_, dbg_flag, &rgen, &hash_map);

            ok_flag &= HashMapUtil<HashMap>::check_forward_iterator(0 /*dvalue*/,
                                                                   dbg_flag, hash_map);
            /* regular forward iterator, but start at hash_map.end() and use operator-- */
            ok_flag &= HashMapUtil<HashMap>::check_backward_iterator(0 /*dvalue*/,
                                                                    dbg_flag, hash_map);

            ok_flag &= HashMapUtil<HashMap>::random_lookups(0 /*dvalue*/,
                                                            dbg_flag, &rgen, hash_map);

            REHEARSE(*p_rh, ok_flag);
        }

        TEST_CASE("DArenaHashMap-try-insert2", "[arena][DArenaHashMap]")
        {
            uint64_t seed = 17747889312058974961ul;

            try_test_array(hashmap_scales(seed),
                           &hashmap_try_insert_test_fn);
        }

        /* insert() grows the table.  A default-constructed map holds one 16-slot group
         * and doubles when load_factor() reaches 0.875, so the scales here climb
         *   16 -> 32 -> 64 -> 128 -> 256 -> 512 slots.
         *
         * Regression: until 2026-08-17 _try_grow() doubled the group *exponent* instead
         * of the group count, so the third rung produced 6 groups rather than 8 -- a
         * capacity that is not a power of 2, while every probe masks with capacity-1.
         * No test grew a table more than twice, so nothing caught it.
         */
        void
        hashmap_grow_test_fn(const TestCase_HashMap & tc,
                             UtestRehearser * p_rh)
        {
            using HashMap = DArenaHashMap<int, int>;

            bool dbg_flag = p_rh->enable_debug();
            bool ok_flag = true;

            /* no capacity hint: we want the growth path */
            HashMap hash_map("utest");

            ok_flag &= HashMapUtil<HashMap>::linear_inserts(tc.n_, dbg_flag, &hash_map);

            ok_flag &= HashMapUtil<HashMap>::check_linear_inserts(tc.n_, dbg_flag, hash_map);

            ok_flag &= HashMapUtil<HashMap>::check_forward_iterator(0 /*dvalue*/,
                                                                   dbg_flag, hash_map);

            REHEARSE(*p_rh, ok_flag);
        }

        TEST_CASE("DArenaHashMap-grow", "[arena][DArenaHashMap]")
        {
            try_test_array(hashmap_scales(0 /*unused: linear_inserts is deterministic*/),
                           &hashmap_grow_test_fn);
        }

        TEST_CASE("DArenaHashMap-operator-bracket", "[arena][DArenaHashMap]")
        {
            scope log(XO_DEBUG_(false));

            using HashMap = DArenaHashMap<int, int>;

            HashMap map("utest");

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

            HashMap map("utest", 1024);

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
