/* @file random_hash_ops.hpp **/

#include <xo/testutil/UtestRehearser.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/randomgen/xoshiro256.hpp>
#include <catch2/catch.hpp>
#include <algorithm>
#include <map>
#include <unordered_set>
#include <vector>

namespace utest {
    struct Util {
        /* generate vector with integers [0.. n-1] */
        [[nodiscard]] static std::vector<std::uint32_t> vector_upto(std::uint32_t n) {
            std::vector<std::uint32_t> u(n);
            for (std::uint32_t i = 0; i < n; ++i)
                u[i] = i;

            return u;
        } /*vector_upto*/

        [[nodiscard]] static std::map<std::uint32_t, std::uint32_t>
        map_upto(std::uint32_t n)
        {
            std::map<std::uint32_t, std::uint32_t> m;
            for(std::uint32_t i=0; i<n; ++i) {
                m[i] = i;
            }

            return m;
        } /*map_upto*/

        /* generate random permutation of integers [0.. n-1] */
        [[nodiscard]] static std::vector<uint32_t>
        random_permutation(uint32_t n, xo::rng::xoshiro256ss *p_rgen) {
            /* vector [0 .. n-1] */
            std::vector<uint32_t> u = vector_upto(n);

            /* shuffle to get unpredictable permutation */
            std::shuffle(u.begin(), u.end(), *p_rgen);

            return u;
        } /*random_permutation*/
    }; /*Util*/

    /* REQUIRE_ORCAPTURE(), REQUIRE_ORFAIL(), REHEARSE() and UtestRehearser come from
     * <xo/testutil/UtestRehearser.hpp>.  They used to be copied into this file; the
     * copies were identical, which is the sort of drift that made them worth sharing.
     *
     * Note UtestTools::bimodal_test() is gone too: it took a std::function, so a test
     * could -- and did -- capture the map and the random generator from the enclosing
     * scope, leaving the diagnostic pass to run against mutated state.  xo::try_test_array()
     * takes a plain function pointer, so the pass function can only see its test case.
     */

    /** what a random_ops() run actually exercised.
     *
     *  Coverage matters here more than usual: the tombstone paths in
     *  _try_insert_aux()/_find()/_try_erase_aux() are unreachable unless a run
     *  both creates tombstones and later inserts over one, and a test that
     *  never reaches them passes just as green as one that does.
     **/
    struct HashMapOpStats {
        /** #of insert operations attempted **/
        std::uint32_t n_insert_ = 0;
        /** #of erase operations that removed a pair **/
        std::uint32_t n_erase_ = 0;
        /** #of erases that left a tombstone (rather than an empty slot) **/
        std::uint32_t n_tombstone_ = 0;
        /** #of inserts that reused a tombstone instead of consuming an empty **/
        std::uint32_t n_reuse_ = 0;
        /** #of inserts that triggered table growth **/
        std::uint32_t n_grow_ = 0;
    };

    /* compare xo-ordinaltree/utest/random_tree_ops.hpp */
    template <typename HashMap>
    struct HashMapUtil : public Util {
#ifdef NOT_YET
        [[nodiscard]] static bool
        test_clear(bool catch_flag,
                   Tree * p_tree)
        {
            bool ok_flag = true;

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->verify_ok(catch_flag));

            p_tree->clear();

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->verify_ok(catch_flag));
            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->empty());
            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->size() == 0);

            return ok_flag;
        } /*test_clear*/
#endif

        [[nodiscard]] static bool
        random_inserts(const std::vector<typename HashMap::key_type> & keys,
                       bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       HashMap * p_map)
        {
            using xo::pp::xtag;

            bool ok_flag = true;

            xo::pp::scope log(XO_DEBUG_(catch_flag), xtag("n-keys", keys.size()));

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->verify_ok(catch_flag));

            /* n keys */
            std::size_t n = keys.size();
            /* permute keys, remembering original position */
            std::vector<std::pair<std::size_t, typename HashMap::key_type>> permuted_keys(n);
            {
                uint32_t i = 0;
                for (const auto & x : keys) {
                    permuted_keys[i] = std::make_pair(i, x);
                }
            }
            /* shuffle to get unpredictable insert order */
            std::shuffle(keys.begin(), keys.end(), *p_rgen);

            size_t tree_z0 = p_map->size();

            /* insert keys in permuted order */
            {
                uint32_t i = 1;
                for(const auto & pr_i : permuted_keys) {
                    log && log(xtag("i", i), xtag("ord", pr_i.first), xtag("n", n), xtag("key", pr_i.second));

                    /* .first:  iterator @ insert position
                     * .second: true if insert occurred (ịẹ tree size incremented)
                     */
                    auto insert_result = p_map->insert(typename HashMap::value_type(pr_i.second, 10.0 * i));

                    REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->verify_ok(catch_flag));

                    REQUIRE_ORFAIL(ok_flag, catch_flag, insert_result.second);

                    /* verify: iterator returned by Treẹinsert(),  refers to inserted key,value pair */
                    log && log(xtag("iter.node", insert_result.first.node()));

                    REQUIRE_ORFAIL(ok_flag, catch_flag, insert_result.first->first == pr_i.second);
                    REQUIRE_ORFAIL(ok_flag, catch_flag, insert_result.first->second == 10.0 * i);

                    ++i;
                }
            }

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->size() == tree_z0 + n);

            return ok_flag;
        }

        /* do
         *   n = (hi - lo) / k
         * random inserts (taken from *p_rgen) into *p_rbtreẹ
         * inserted keys will comprise the distinct values
         *   {lo, lo+k, lo+2k, ..., lo+n.k}
         */
        [[nodiscard]] static bool
        random_inserts(std::uint32_t lo,
                       std::uint32_t hi,
                       std::uint32_t k,
                       bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       HashMap * p_map)
        {
            // TODO: rewrite in terms of 'random_inserts with explicit vector'.

            using xo::pp::xtag;

            bool ok_flag = true;

            xo::pp::scope log(XO_DEBUG_(catch_flag), xtag("lo", lo), xtag("hi", hi), xtag("k", k));

            auto policy = xo::verify_policy::chatty();

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->verify_ok(policy));

            if ((hi <= lo) || (k == 0))
                return true;

            uint32_t n = (hi - lo) / k;

            /* n keys 0..n-1 */
            std::vector<std::uint32_t> u(n);
            for(std::uint32_t i=0; i<n; ++i)
                u[i] = lo + i*k;

            /* shuffle to get unpredictable insert order */
            std::shuffle(u.begin(), u.end(), *p_rgen);

            size_t tree_z0 = p_map->size();

            /* insert keys according to permutation u */
            uint32_t i = 1;
            for(uint32_t x : u) {
                log && log(xtag("i", i), xtag("n", n), xtag("key", x));
                /* .first:  iterator @ insert position
                 * .second: true if insert occurred (ịẹ tree size incremented)
                 */
                auto insert_result = p_map->try_insert(typename HashMap::value_type(x, 10 * x));

                REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->verify_ok(policy));

                REQUIRE_ORFAIL(ok_flag, catch_flag, insert_result.second);

                /* verify: iterator returned by Treẹinsert(),  refers to inserted key,value pair */
                //log && log(xtag("iter.node", insert_result.first.node()));
                REQUIRE_ORFAIL(ok_flag, catch_flag, insert_result.first->first == x);
                REQUIRE_ORFAIL(ok_flag, catch_flag, insert_result.first->second == 10 * x);

                ++i;
            }

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->size() == tree_z0 + n);

            return ok_flag;
        } /*random_inserts*/

        [[nodiscard]] static bool
        random_inserts(std::uint32_t n,
                       bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       HashMap * p_map)
        {
            return random_inserts(0, n, 1, catch_flag, p_rgen, p_map);
        }

        /* insert the keys {0, 1, .., n-1} into *p_map, in order,
         * with value 10*key (the dvalue=0 case of the convention the
         * check_.._iterator() helpers assume).
         *
         * Uses insert(), not try_insert(), so the table grows: this is the
         * only helper here that exercises DArenaHashMap::_try_grow().
         * Table state is verified after each insert, since a growth that
         * corrupts it (see DArenaHashMap-grow) is otherwise invisible until
         * a later lookup misses.
         */
        [[nodiscard]] static bool
        linear_inserts(std::uint32_t n,
                       bool catch_flag,
                       HashMap * p_map)
        {
            using xo::pp::scope;
            using xo::pp::xtag;

            bool ok_flag = true;

            scope log(XO_DEBUG_(catch_flag), xtag("n", n));

            /* first pass wants a verdict, not a diagnostic: the silent policy returns
             * false instead of logging + throwing, which is what lets bimodal_test()
             * rerun the failing scale with logging on
             */
            auto policy = (catch_flag
                           ? xo::verify_policy::chatty()
                           : xo::verify_policy());

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->verify_ok(policy));

            std::size_t map_z0 = p_map->size();

            for (std::uint32_t i = 0; i < n; ++i) {
                auto key = static_cast<typename HashMap::key_type>(i);

                log && log(xtag("i", i), xtag("key", key),
                           xtag("size", p_map->size()),
                           xtag("capacity", p_map->capacity()));

                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               p_map->insert(typename HashMap::value_type(key, 10 * key)));

                /* SM1.3 (n_group_ vs n_group_exponent_) and SM1.5 (n_slot_ a power of 2)
                 * live here; a growth that computes the wrong group count trips them
                 */
                REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->verify_ok(policy));

                REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->size() == map_z0 + i + 1);

                std::size_t cap = p_map->capacity();

                REQUIRE_ORFAIL(ok_flag, catch_flag, cap == p_map->groups() * HashMap::c_group_size);
                /* probe arithmetic masks with (capacity - 1), so this must hold */
                REQUIRE_ORFAIL(ok_flag, catch_flag, (cap > 0) && ((cap & (cap - 1)) == 0));
                REQUIRE_ORFAIL(ok_flag, catch_flag,
                               p_map->load_factor() <= HashMap::c_max_load_factor);
            }

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->size() == map_z0 + n);

            return ok_flag;
        } /*linear_inserts*/

        /** count tombstone control bytes in @p map **/
        [[nodiscard]] static std::uint32_t
        count_tombstones(HashMap * p_map)
        {
            std::uint32_t n = 0;
            auto * p_store = p_map->_store();

            for (std::size_t i = 0, N = p_map->capacity(); i < N; ++i) {
                if (p_store->control_[HashMap::c_control_stub + i] == HashMap::c_tombstone)
                    ++n;
            }

            return n;
        } /*count_tombstones*/

        /** Apply @p n_op random insert/erase operations, drawn from the key range
         *  [0, key_space), with @p erase_pct percent of them erases.  Keeps a
         *  std::map alongside as a reference model and checks the table against it.
         *
         *  Values follow the dvalue=0 convention (value = 10*key), so the
         *  check_..._iterator() helpers compose with this.
         *
         *  Per operation: the affected key behaves (present after insert, absent
         *  after erase), the erase return code matches the model, and size()
         *  agrees.  Every c_sweep_period operations, and once at the end: every
         *  model key is findable with the right value, and verify_ok() passes.
         *  The periodic sweep is what catches a severed probe chain -- a key that
         *  was inserted long ago and became unreachable later.
         **/
        [[nodiscard]] static bool
        random_ops(std::uint32_t n_op,
                   std::uint32_t key_space,
                   std::uint32_t erase_pct,
                   bool catch_flag,
                   xo::rng::xoshiro256ss * p_rgen,
                   HashMap * p_map,
                   HashMapOpStats * p_stats)
        {
            using xo::pp::scope;
            using xo::pp::xtag;

            constexpr std::uint32_t c_sweep_period = 64;

            bool ok_flag = true;

            scope log(XO_DEBUG_(catch_flag),
                      xtag("n_op", n_op), xtag("key_space", key_space),
                      xtag("erase_pct", erase_pct));

            auto policy = (catch_flag
                           ? xo::verify_policy::chatty()
                           : xo::verify_policy());

            /* reference model */
            std::map<typename HashMap::key_type, typename HashMap::mapped_type> model;

            for (std::uint32_t i_op = 0; i_op < n_op; ++i_op) {
                auto key = static_cast<typename HashMap::key_type>((*p_rgen)() % key_space);
                bool erase_op = (((*p_rgen)() % 100) < erase_pct);

                std::size_t cap0 = p_map->capacity();
                std::uint32_t tomb0 = count_tombstones(p_map);

                if (erase_op) {
                    bool present = (model.find(key) != model.end());

                    std::size_t n_erased = p_map->erase(key);
                    model.erase(key);

                    REQUIRE_ORFAIL(ok_flag, catch_flag, n_erased == (present ? 1u : 0u));
                    REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->find(key) == p_map->end());

                    if (present) {
                        ++(p_stats->n_erase_);

                        if (count_tombstones(p_map) > tomb0)
                            ++(p_stats->n_tombstone_);
                    }
                } else {
                    p_map->insert(typename HashMap::value_type(key, 10 * key));
                    model[key] = 10 * key;

                    ++(p_stats->n_insert_);

                    if (p_map->capacity() != cap0) {
                        ++(p_stats->n_grow_);
                    } else if (count_tombstones(p_map) < tomb0) {
                        /* size grew without consuming an empty slot:
                         * _try_insert_aux() reused a tombstone
                         */
                        ++(p_stats->n_reuse_);
                    }

                    auto ix = p_map->find(key);

                    REQUIRE_ORFAIL(ok_flag, catch_flag, ix != p_map->end());
                    REQUIRE_ORFAIL(ok_flag, catch_flag, ix->second == 10 * key);
                }

                REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->size() == model.size());

                if (((i_op % c_sweep_period) == 0) || (i_op + 1 == n_op)) {
                    REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->verify_ok(policy));

                    /* every surviving key is still reachable.  This is what
                     * catches a severed probe chain: an entry inserted long ago
                     * that a later erase made unreachable.
                     */
                    for (const auto & kv : model) {
                        auto jx = p_map->find(kv.first);

                        REQUIRE_ORFAIL(ok_flag, catch_flag, jx != p_map->end());
                        REQUIRE_ORFAIL(ok_flag, catch_flag, jx->second == kv.second);
                    }

                    /* iteration visits exactly the surviving keys -- i.e. it skips
                     * tombstones as well as empty slots.  Cannot use
                     * check_forward_iterator() here: that assumes keys are 0..n-1.
                     */
                    std::size_t n_visit = 0;

                    for (const auto & kv : *p_map) {
                        auto mx = model.find(kv.first);

                        REQUIRE_ORFAIL(ok_flag, catch_flag, mx != model.end());
                        REQUIRE_ORFAIL(ok_flag, catch_flag, mx->second == kv.second);

                        ++n_visit;
                    }

                    REQUIRE_ORFAIL(ok_flag, catch_flag, n_visit == model.size());
                }
            }

            log && log("done",
                       xtag("n_insert", p_stats->n_insert_),
                       xtag("n_erase", p_stats->n_erase_),
                       xtag("n_tombstone", p_stats->n_tombstone_),
                       xtag("n_reuse", p_stats->n_reuse_),
                       xtag("n_grow", p_stats->n_grow_));

            return ok_flag;
        } /*random_ops*/

        /* verify the keys {0, 1, .., n-1} are all present with value 10*key.
         * Complements linear_inserts(): a table left with a capacity that is not a
         * power of 2 has slots no probe can reach, so keys go missing here.
         */
        [[nodiscard]] static bool
        check_linear_inserts(std::uint32_t n,
                             bool catch_flag,
                             HashMap & map)
        {
            using xo::pp::scope;
            using xo::pp::xtag;

            bool ok_flag = true;

            scope log(XO_DEBUG_(catch_flag), xtag("n", n));

            REQUIRE_ORFAIL(ok_flag, catch_flag, map.size() == n);

            for (std::uint32_t i = 0; i < n; ++i) {
                auto key = static_cast<typename HashMap::key_type>(i);

                auto ix = map.find(key);

                log && log(xtag("i", i), xtag("key", key), xtag("found", ix != map.end()));

                REQUIRE_ORFAIL(ok_flag, catch_flag, ix != map.end());
                REQUIRE_ORFAIL(ok_flag, catch_flag, ix->first == key);
                REQUIRE_ORFAIL(ok_flag, catch_flag, ix->second == 10 * key);
            }

            return ok_flag;
        } /*check_linear_inserts*/

#ifdef NOT_YET
        /* do n random removes (taken from *p_rgen) from *p_rbtree;
         * assumes *p_rbtree has keys [0 .. n-1] where n=p_rbtreẹsize
         */
        [[nodiscard]] static bool
        random_removes(bool catch_flag, // dbg_flag
                       xo::rng::xoshiro256ss * p_rgen,
                       Tree * p_map)
        {
            using xo::pp::scope;
            using xo::pp::xtag;

            bool ok_flag = true;

            xo::pp::scope log(XO_DEBUG_(catch_flag));

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->verify_ok(catch_flag));

            uint32_t n = p_map->size();

            /* random permutation of keys in *p_map */
            std::vector<std::uint32_t> u
                = random_permutation(n, p_rgen);

            log && log(xtag("remove-order", u));

            /* will keep track of which keys remain as we move them */
            std::map<std::uint32_t, std::uint32_t> m = Util::map_upto(n);

            /* remove keys in permutation order */
            std::uint32_t i = 1;
            for (std::uint32_t x : u) {
                log && log("iter i: removing key from n-node tree",
                           xtag("i", i), xtag("key", x), xtag("n", n));

                /* remove x from tracking map m also */
                m.erase(x);

                log && log("remove key :iter ", i, "/", n, xtag("key", x));

                p_map->erase(x);
                // rbtreẹdisplay();
                REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->size() == n-i);
                /* amongst other things,  this guarantees that keys in *p_map
                 * appear in increasing order
                 */
                REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->verify_ok(catch_flag));

#ifdef NOT_YET
                /* 1. rbtree should now contain all the keys in [0..n-1],
                 *    with u[0]..u[i-1] excluded;  this is the same as the
                 *    contents of m.
                 */
                auto m_ix = m.begin();
                auto m_end_ix = m.end();
                auto visitor_fn =
                    ([&m_ix, m_end_ix]
                     (std::pair<int, double> const & contents)
                    {
                        REQUIRE(m_ix != m_end_ix);
                        REQUIRE(contents.first == m_ix->second);
                        ++m_ix;
                    });
                p_map->visit_inorder(visitor_fn);
#endif
                ++i;
            }

            REQUIRE_ORFAIL(ok_flag, catch_flag, m.empty());
            REQUIRE_ORFAIL(ok_flag, catch_flag, p_map->size() == 0);

            log.end_scope();

            return ok_flag;
        } /*random_removes*/
#endif

        /* Require:
         * - map has keys [0..n-1], where n=map.size()
         * - for each key k, associated value is dvalue+10*k
         */
        [[nodiscard]] static bool
        random_lookups(uint32_t dvalue,
                       bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       HashMap & map)
        {
            using xo::pp::scope;
            using xo::pp::xtag;

            xo::pp::scope log(XO_DEBUG_(catch_flag));

            /* -> false if/when verification fails */
            bool ok_flag = true;

            REQUIRE_ORFAIL(ok_flag, catch_flag, map.verify_ok());

            size_t n = map.size();
            std::vector<std::uint32_t> u
                = random_permutation(n, p_rgen);

            /* lookup keys in permutation order */
            std::uint32_t i = 1;
            for (std::uint32_t x : u) {
                INFO(tostr0(xtag("i", i), xtag("n", n), xtag("x", x)));

                auto find_ix = map.find(x);

                REQUIRE_ORFAIL(ok_flag, catch_flag, find_ix != map.end());
                REQUIRE_ORFAIL(ok_flag, catch_flag, find_ix->first == x);
                REQUIRE_ORFAIL(ok_flag, catch_flag, find_ix->second == dvalue + x*10);
                REQUIRE_ORFAIL(ok_flag, catch_flag, map.verify_ok());
                REQUIRE_ORFAIL(ok_flag, catch_flag, map.size() == n);

                ++i;
            }

            REQUIRE_ORFAIL(ok_flag, catch_flag, map.size() == n);

            log.end_scope();

            return ok_flag;
        } /*random_lookups*/

        /* Require:
         * - hash has keys [0..n-1] where n=map size
         * - hash value at key k is dvalue+10*k
         */
        [[nodiscard]] static bool
        check_forward_iterator(uint32_t dvalue,
                               bool catch_flag,
                               HashMap & map)
        {
            using xo::pp::scope;
            using xo::pp::xtag;

            /* -> flase if/when verification fails */
            bool ok_flag = true;

            std::size_t const n = map.size();

            scope log(XO_DEBUG_(catch_flag));

            log && log("map with size n", xtag("n", n));

            std::unordered_set<std::size_t> keys;

            {
                auto end_ix = map.end();

                //log && log(xtag("end_ix", end_ix));

                auto begin_ix = map.begin();
                auto ix = begin_ix;

                int last_key = -1;

                while (ix != end_ix) {
                    log && log("forward loop top"
                               //xtag("ix", ix)
                               );

                    /* verify: keys in map are in [0 .. n) */
                    REQUIRE_ORFAIL(ok_flag, catch_flag, 0 <= ix->first);
                    REQUIRE_ORFAIL(ok_flag, catch_flag, ix->first < n);

                    /* verify: keys in map are unique */
                    REQUIRE_ORFAIL(ok_flag, catch_flag, !keys.contains(ix->first));
                    keys.insert(ix->first);

                    REQUIRE_ORFAIL(ok_flag, catch_flag, ix->second == dvalue + 10 * ix->first);

                    last_key = ix->first;
                    ++ix;

                    log && log("forward loop bottom",
                               xtag("last_key", last_key)
                               //xtag("next ix", ix)
                               );
                }

                /* should have visited exactly n locations */
                REQUIRE_ORFAIL(ok_flag, catch_flag, map.size() == keys.size());
                REQUIRE_ORFAIL(ok_flag, catch_flag, ix == end_ix);

                //log && log(xtag("ix", ix), xtag("begin_ix", begin_ix));
            }

            return ok_flag;
        }

        /* Require:
         * - hash has keys [0..n-1] where n=map size
         * - hash value at key k is dvalue+10*k
         */
        [[nodiscard]] static bool
        check_backward_iterator(uint32_t dvalue,
                                bool catch_flag,
                                HashMap & map)
        {
            using xo::pp::scope;
            using xo::pp::xtag;

            /* -> flase if/when verification fails */
            bool ok_flag = true;

            std::size_t const n = map.size();

            scope log(XO_DEBUG_(catch_flag));

            log && log("map with size n", xtag("n", n));

            std::unordered_set<std::size_t> keys;

            {
                auto end_ix = map.end();

                //log && log(xtag("end_ix", end_ix));

                auto begin_ix = map.begin();
                auto ix = end_ix;

                if (ix == begin_ix) [[unlikely]] {
                    return ok_flag;
                }

                while (ix != begin_ix) {
                    log && log("backward loop top",
                               xtag("n", n)
                               );

                    --ix;

                    /* verify: keys in map are in [0 .. n) */
                    REQUIRE_ORFAIL(ok_flag, catch_flag, 0 <= ix->first);
                    REQUIRE_ORFAIL(ok_flag, catch_flag, ix->first < n);

                    log && log(xtag("ix->first", ix->first));

                    /* verify: keys in map are unique */
                    REQUIRE_ORFAIL(ok_flag, catch_flag, !keys.contains(ix->first));
                    keys.insert(ix->first);

                    REQUIRE_ORFAIL(ok_flag, catch_flag, ix->second == dvalue + 10 * ix->first);
                }

                /* should have visited exactly n locations */
                REQUIRE_ORFAIL(ok_flag, catch_flag, map.size() == keys.size());
                REQUIRE_ORFAIL(ok_flag, catch_flag, ix == begin_ix);

                //log && log(xtag("ix", ix), xtag("begin_ix", begin_ix));
            }

            return ok_flag;
        }

#ifdef NOT_YET
        /* Require:
         * - tree has keys [0..n-1], where n=treẹsize()
         * - tree values at key k is dvalue+10*k
         *
         * catch_flag.  true -> log to console + interact with catch2
         *              false -> verify iteration behavior for return code
         */
        [[nodiscard]] static bool
        check_bidirectional_iterator(uint32_t dvalue,
                                     bool catch_flag,
                                     Tree const & tree)
        {
            using xo::pp::scope;
            using xo::pp::xtag;

            /* -> false if/when verification fails */
            bool ok_flag = true;

            std::size_t const n = tree.size();

            xo::pp::scope log(XO_DEBUG_(catch_flag));

            log && log("tree with size n", xtag("n", n));

            {
                std::size_t i = 0;

                auto end_ix = tree.end();

                log && log(xtag("end_ix", end_ix));

                auto begin_ix = tree.begin();
                auto ix = begin_ix;

                int last_key = -1;

                while (ix != end_ix) {
                    log && log("forward loop top",
                               xtag("i", i),
                               xtag("ix", ix));

                    REQUIRE_ORFAIL(ok_flag, catch_flag, ix->first == i);
                    REQUIRE_ORFAIL(ok_flag, catch_flag, ix->second == dvalue + 10*i);
                    if(i > 0) {
                        REQUIRE_ORFAIL(ok_flag, catch_flag, ix->first > last_key);
                    }
                    last_key = ix->first;
                    ++i;
                    ++ix;

                    log && log("forward loop bottom",
                               xtag("last_key", last_key),
                               xtag("next ix", ix));
                }

                /* should have visited exactly n locations */
                REQUIRE_ORFAIL(ok_flag, catch_flag, i == n);
                REQUIRE_ORFAIL(ok_flag, catch_flag, ix == end_ix);

                log && log(xtag("ix", ix), xtag("begin_ix", begin_ix));

                /* now run iterator backwards,
                 * starting from "one past the end"
                 */
                if(ix != begin_ix) {
                    do {
                        --i;
                        --ix;

                        log && log("forward backup",
                                   xtag("i", i),
                                   xtag("ix", ix));

                        REQUIRE_ORFAIL(ok_flag, catch_flag, ix.is_dereferenceable());

                        log && log(xtag("ix.first", (*ix).first));

                        REQUIRE_ORFAIL(ok_flag, catch_flag, (*ix).first == i);
                    } while (ix != begin_ix);
                }

                /* should have visited exactly n locations in reverse */
                REQUIRE_ORFAIL(ok_flag, catch_flag, i == 0);
            }

            /* ----- reverse iterators ----- */

            {
                std::int64_t i = n - 1;

                auto rbegin_ix = tree.rbegin();
                auto rend_ix = tree.rend();

                auto rix = rbegin_ix;

                int last_key = -1;

                while (rix != rend_ix) {
                    log && log("reverse loop top",
                               xtag("i", i),
                               xtag("rix", rix));

                    REQUIRE_ORFAIL(ok_flag, catch_flag, rix->first == i);
                    REQUIRE_ORFAIL(ok_flag, catch_flag, rix->second == dvalue + 10*i);
                    if (i < n-1) {
                        REQUIRE_ORFAIL(ok_flag, catch_flag, rix->first < last_key);
                    }
                    last_key = rix->first;
                    --i;
                    ++rix;

                    log && log("reverse loop bottom",
                               xtag("last_key", last_key),
                               xtag("next ix", rix));
                }

                /* should have visited exactly n locations */
                REQUIRE_ORFAIL(ok_flag, catch_flag, i == -1);

                log && log(xtag("rbegin_ix", rbegin_ix));

                /* now run reverse iterator backwrds,
                 * starting from "one before the beginning"
                 */
                if (rix != rbegin_ix) {
                    do {
                        ++i;
                        --rix;

                        log && log("reverse backup",
                                   xtag("i", i),
                                   xtag("rix", rix),
                                   xtag("rix.first", rix->first));

                        REQUIRE_ORFAIL(ok_flag, catch_flag, (*rix).first == i);
                    } while (rix != rbegin_ix);
                }

                /* should have visited exactly n locations in reversê2 */
                REQUIRE_ORFAIL(ok_flag, catch_flag, i == n - 1);
            }

            log.end_scope();

            return ok_flag;
        } /*check_bidirectional_iterator*/
#endif

#ifdef NOT_YET
        /* Require:
         * - *p_rbtree has keys [0..n-1],  where n=rbtree.size()
         * - for each key k,  associated value is 10*k
         *
         * Promise:
         * - for each key k,  associated value is dvalue + 10*k
         */
        [[nodiscard]] static bool
        random_updates(uint32_t dvalue,
                       bool catch_flag,
                       Tree * p_rbtree,
                       xo::rng::xoshiro256ss * p_rgen)
        {
            using xo::pp::scope;
            using xo::pp::xtag;

            scope log(XO_DEBUG_(catch_flag));

            /* -> false if/when check fails */
            bool ok_flag = true;

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_rbtree->verify_ok());

            std::size_t n = p_rbtree->size();
            std::vector<uint32_t> u
                = Util::random_permutation(n, p_rgen);

            /* update key/value pairs in permutation order */
            uint32_t i = 1;
            for (uint32_t x : u) {
                REQUIRE_ORFAIL(ok_flag, catch_flag, (*p_rbtree)[x] == x*10);

                (*p_rbtree)[x] = dvalue + 10*x;

                REQUIRE_ORFAIL(ok_flag, catch_flag, (*p_rbtree)[x] == dvalue + 10*x);
                REQUIRE_ORFAIL(ok_flag, catch_flag, p_rbtree->verify_ok());
                /* assignment to existing key does not change tree size */
                REQUIRE_ORFAIL(ok_flag, catch_flag, p_rbtree->size() == n);
                ++i;
            }

            REQUIRE(p_rbtree->size() == n);

            return ok_flag;
        } /*random_updates*/
#endif
    }; /*TreeUtil*/
} /*namespace utest*/

/* end random_tree_ops.hpp */
