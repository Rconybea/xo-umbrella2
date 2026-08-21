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
#include <vector>
#include <string>

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

        /* ---------------------------------------------------------------------
         * HashMapStore::_needs_tombstone(ix)
         *
         * Answers: after erasing the entry at slot ix, must the control byte become
         * a tombstone rather than empty?  It must exactly when some 16-wide probe
         * window containing ix is entirely non-empty -- i.e. when a probe could have
         * walked past ix -- which is equivalent to: the maximal run of consecutive
         * non-empty control bytes through ix is at least c_group_size long.
         *
         * The reference below computes that run directly, cyclically, and is the
         * oracle; _needs_tombstone() computes it from two SIMD-shaped group loads
         * and two bit counts.  The two must agree for every occupancy pattern.
         *
         * Note this test writes control bytes without touching size_, so the store
         * does NOT satisfy the class invariants -- do not call verify_ok() here.
         * ------------------------------------------------------------------------- */
        struct TestCase_Tombstone {
            /* number of 16-slot groups.  Must be a power of 2 (SM1.3), so 1, 2, 4 --
             * there is no valid 3-group table to test.
             */
            std::uint32_t n_group_ = 1;
            /* enumerate every occupancy pattern; only feasible for a single group */
            bool exhaustive_ = false;
            /* otherwise, this many pseudo-random patterns */
            std::uint32_t n_random_ = 0;
            std::uint64_t seed_ = 0;
        };

        using TombstoneStore = xo::map::detail::HashMapStore<int, int>;

        /** the property, computed directly: length of the run of non-empty control
         *  bytes through ix, walking both ways cyclically, capped at c_group_size
         **/
        static bool
        tombstone_ref(const std::vector<std::uint8_t> & ctrl_v, std::size_t ix)
        {
            constexpr auto G = DArenaHashMapUtil::c_group_size;
            const std::size_t N = ctrl_v.size();

            std::size_t run = 1;                       /* ix itself, known occupied */

            for (std::size_t k = 1; (k <= G) && (run < G); ++k) {
                if (ctrl_v[(ix + N - k) % N] == DArenaHashMapUtil::c_empty_slot)
                    break;
                ++run;
            }
            for (std::size_t k = 1; (k <= G) && (run < G); ++k) {
                if (ctrl_v[(ix + k) % N] == DArenaHashMapUtil::c_empty_slot)
                    break;
                ++run;
            }

            return run >= G;
        }

        /** install ctrl_v into the store (via _update_control, so the wrap copy is
         *  refreshed), then compare _needs_tombstone() against the oracle at every
         *  occupied slot.  On disagreement report the slot in *p_fail_ix.
         **/
        static bool
        tombstone_check_pattern(TombstoneStore * p_store,
                                const std::vector<std::uint8_t> & ctrl_v,
                                std::size_t * p_fail_ix)
        {
            for (std::size_t i = 0, n = ctrl_v.size(); i < n; ++i)
                p_store->_update_control(i, ctrl_v[i]);

            for (std::size_t ix = 0, n = ctrl_v.size(); ix < n; ++ix) {
                if (ctrl_v[ix] == DArenaHashMapUtil::c_empty_slot)
                    continue;                          /* erase applies to occupied slots */

                if (p_store->_needs_tombstone(ix) != tombstone_ref(ctrl_v, ix)) {
                    *p_fail_ix = ix;
                    return false;
                }
            }

            return true;
        }

        static std::string
        tombstone_render(const std::vector<std::uint8_t> & ctrl_v)
        {
            std::string s;
            for (auto c : ctrl_v) {
                s += (c == DArenaHashMapUtil::c_empty_slot
                      ? '.'
                      : (c == DArenaHashMapUtil::c_tombstone ? 'x' : '#'));
            }
            return s;
        }

        void
        needs_tombstone_test_fn(const TestCase_Tombstone & tc,
                                UtestRehearser * p_rh)
        {
            using xo::rng::xoshiro256ss;

            bool dbg_flag = p_rh->enable_debug();
            scope log(XO_DEBUG2_(dbg_flag, "needs_tombstone"));

            const std::size_t N = tc.n_group_ * DArenaHashMapUtil::c_group_size;
            const std::uint8_t EMPTY = DArenaHashMapUtil::c_empty_slot;
            const std::uint8_t TOMB = DArenaHashMapUtil::c_tombstone;

            TombstoneStore store("utest", DArenaHashMapUtil::lub_exp2(tc.n_group_));

            std::vector<std::vector<std::uint8_t>> pattern_v;

            /* targeted: runs of exactly 15 (never a tombstone) and exactly 16 (always,
             * for every slot in the run) at every start offset, so the runs that
             * straddle the N-1 -> 0 boundary -- the wrap-copy path -- are all covered
             */
            for (std::size_t run : {DArenaHashMapUtil::c_group_size - 1,
                                    DArenaHashMapUtil::c_group_size}) {
                for (std::size_t start = 0; start < N; ++start) {
                    std::vector<std::uint8_t> v(N, EMPTY);
                    for (std::size_t k = 0; k < run; ++k)
                        v[(start + k) % N] = static_cast<std::uint8_t>(k & 0x7f);
                    pattern_v.push_back(v);
                }
            }

            /* every slot occupied, and every slot occupied but one */
            pattern_v.push_back(std::vector<std::uint8_t>(N, std::uint8_t(0x11)));
            for (std::size_t hole = 0; hole < N; ++hole) {
                std::vector<std::uint8_t> v(N, std::uint8_t(0x22));
                v[hole] = EMPTY;
                pattern_v.push_back(v);
            }

            if (tc.exhaustive_) {
                /* N == 16: enumerate all 2^16 occupancy patterns */
                for (std::uint32_t bits = 0; bits < (1u << N); ++bits) {
                    std::vector<std::uint8_t> v(N, EMPTY);
                    for (std::size_t i = 0; i < N; ++i) {
                        if (bits & (1u << i))
                            v[i] = static_cast<std::uint8_t>(i & 0x7f);
                    }
                    pattern_v.push_back(v);
                }
            } else {
                /* random, across densities, with tombstones mixed in: they are
                 * non-empty for this purpose and must lengthen a run
                 */
                auto rgen = xoshiro256ss(tc.seed_);
                for (std::uint32_t i = 0; i < tc.n_random_; ++i) {
                    std::uint32_t pct = 40 + (rgen() % 60);        /* 40..99 % full */
                    std::vector<std::uint8_t> v(N, EMPTY);
                    for (std::size_t j = 0; j < N; ++j) {
                        if ((rgen() % 100) < pct) {
                            v[j] = ((rgen() % 8) == 0)
                                ? TOMB
                                : static_cast<std::uint8_t>(rgen() & 0x7f);
                        }
                    }
                    pattern_v.push_back(v);
                }
            }

            log && log(xtag("n_group", tc.n_group_), xtag("N", N),
                       xtag("patterns", pattern_v.size()));

            bool ok_flag = true;
            std::size_t fail_ix = 0;

            for (const auto & ctrl_v : pattern_v) {
                if (!tombstone_check_pattern(&store, ctrl_v, &fail_ix)) {
                    ok_flag = false;
                    log && log("mismatch",
                               xtag("N", N),
                               xtag("ix", fail_ix),
                               xtag("expect", tombstone_ref(ctrl_v, fail_ix)),
                               xtag("actual", store._needs_tombstone(fail_ix)),
                               xtag("ctrl", tombstone_render(ctrl_v)));
                    break;
                }
            }

            REHEARSE(*p_rh, ok_flag);
        }

        TEST_CASE("HashMapStore-needs-tombstone", "[arena][DArenaHashMap]")
        {
            std::vector<TestCase_Tombstone> tc_v = {
                TestCase_Tombstone{.n_group_ = 1, .exhaustive_ = true},
                TestCase_Tombstone{.n_group_ = 2, .n_random_ = 20000, .seed_ = 8675309ul},
                TestCase_Tombstone{.n_group_ = 4, .n_random_ = 20000, .seed_ = 20260819ul},
            };

            try_test_array(tc_v, &needs_tombstone_test_fn);
        }

        /* ---------------------------------------------------------------------
         * Randomized insert/erase/lookup against a std::map reference model.
         *
         * This is the only case that reaches the tombstone paths through the
         * public API, so it also asserts COVERAGE: a run that never created a
         * tombstone, or never reused one, proves nothing about the code it is
         * meant to protect, and would otherwise pass silently.
         *
         * Two hash regimes, because they stress different things:
         *  - std::hash<int> is the identity, so h1 = key >> 7 makes keys cluster
         *    into few probe starts: long runs, many tombstones, deep probes.
         *  - a mixing hash scatters keys: tombstones are sparser and reuse is
         *    rarer, which is the ordinary case.
         * ------------------------------------------------------------------------- */
        struct MixHash {
            std::size_t operator()(int k) const {
                std::uint64_t x = static_cast<std::uint64_t>(k) + 0x9E3779B97F4A7C15ull;
                x ^= (x >> 30); x *= 0xBF58476D1CE4E5B9ull;
                x ^= (x >> 27); x *= 0x94D049BB133111EBull;
                return static_cast<std::size_t>(x ^ (x >> 31));
            }
        };

        struct TestCase_RandomOps {
            std::uint32_t n_op_ = 0;
            /** keys drawn from [0, key_space_) **/
            std::uint32_t key_space_ = 0;
            /** percent of operations that are erases **/
            std::uint32_t erase_pct_ = 0;
            /** true to use a mixing hash instead of std::hash<int> **/
            bool mixed_hash_ = false;
            std::uint64_t seed_ = 0;
        };

        template <typename HashMap>
        static bool
        run_random_ops(const TestCase_RandomOps & tc,
                       bool dbg_flag,
                       utest::HashMapOpStats * p_stats)
        {
            using xo::rng::xoshiro256ss;

            bool ok_flag = true;

            /* fresh for each pass -- see DArenaHashMap-try-insert2 */
            auto rgen = xoshiro256ss(tc.seed_);
            HashMap hash_map("utest");

            ok_flag &= utest::HashMapUtil<HashMap>::random_ops(tc.n_op_, tc.key_space_,
                                                               tc.erase_pct_, dbg_flag,
                                                               &rgen, &hash_map, p_stats);

            return ok_flag;
        }

        void
        random_ops_test_fn(const TestCase_RandomOps & tc,
                           UtestRehearser * p_rh)
        {
            bool dbg_flag = p_rh->enable_debug();
            scope log(XO_DEBUG2_(dbg_flag, "random_ops"));

            utest::HashMapOpStats stats;

            bool ok_flag = (tc.mixed_hash_
                            ? run_random_ops<DArenaHashMap<int, int, MixHash>>(tc, dbg_flag, &stats)
                            : run_random_ops<DArenaHashMap<int, int>>(tc, dbg_flag, &stats));

            log && log(xtag("n_insert", stats.n_insert_),
                       xtag("n_erase", stats.n_erase_),
                       xtag("n_tombstone", stats.n_tombstone_),
                       xtag("n_reuse", stats.n_reuse_),
                       xtag("n_grow", stats.n_grow_));

            REHEARSE(*p_rh, ok_flag);

            /* coverage: the run must have exercised what it exists to exercise */
            REHEARSE(*p_rh, stats.n_erase_ > 0);
            REHEARSE(*p_rh, stats.n_tombstone_ > 0);
            REHEARSE(*p_rh, stats.n_reuse_ > 0);
        }

        TEST_CASE("DArenaHashMap-random-ops", "[arena][DArenaHashMap]")
        {
            std::vector<TestCase_RandomOps> tc_v = {
                TestCase_RandomOps{.n_op_ = 2000, .key_space_ = 200, .erase_pct_ = 40,
                                   .mixed_hash_ = false, .seed_ = 5150ul},
                TestCase_RandomOps{.n_op_ = 2000, .key_space_ = 200, .erase_pct_ = 40,
                                   .mixed_hash_ = true,  .seed_ = 90210ul},
                /* erase-heavy: churns tombstones without growing much */
                TestCase_RandomOps{.n_op_ = 2000, .key_space_ = 64, .erase_pct_ = 55,
                                   .mixed_hash_ = false, .seed_ = 20260821ul},
            };

            try_test_array(tc_v, &random_ops_test_fn);
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

        TEST_CASE("DArenaHashMap-find-past-tombstone", "[arena][DArenaHashMap]")
        {
            using HashMap = DArenaHashMap<int, int>;

            /* std::hash<int> is the identity, so h1 = key >> 7: keys 0..127 all
             * probe from the same start, filling slots contiguously.
             */
            HashMap map("utest");

            constexpr int c_n = 28;
            for (int k = 0; k < c_n; ++k)
                map.insert(std::make_pair(k, 10 * k));

            REQUIRE(map.size() == c_n);

            /* a key that lives beyond the first 16-slot probe window */
            int far_key = 20;
            REQUIRE(map.find(far_key) != map.end());

            /* erase an early key.  The run of occupied slots through it is >= 16,
             * so _needs_tombstone() must choose a tombstone rather than an empty.
             *
             * Asserted directly: if this ever became c_empty_slot, the far_key
             * lookup below would fail for a confusing reason (its probe would
             * stop at the empty) rather than pointing at the erase.
             *
             * Note the run exists because std::hash<int> is the identity in
             * libstdc++, so keys 0..127 share a probe start and fill slots
             * contiguously.  That is a libstdc++ property, not a guarantee --
             * the control-byte check below is what keeps this test honest if
             * it ever changes.
             */
            REQUIRE(map.erase(5) == 1);
            REQUIRE(map._store()->control_[DArenaHashMapUtil::c_control_stub + 5]
                    == DArenaHashMapUtil::c_tombstone);

            /* the rest of erase's contract */
            REQUIRE(map.size() == c_n - 1);
            REQUIRE(map.find(5) == map.end());   /* erased key is gone */
            REQUIRE(map.erase(5) == 0);          /* erasing it again is a no-op */
            REQUIRE(map.erase(999) == 0);        /* key that was never present */

            /* the far key is still present, and must still be findable:
             * a probe has to walk *past* the tombstone to reach it
             */
            auto ix = map.find(far_key);
            REQUIRE(ix != map.end());
            REQUIRE(ix->second == 10 * far_key);
        }
    }
}

/* end DArenaHashMap.test.cpp */
