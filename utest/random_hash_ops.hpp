/* @file random_hash_ops.hpp **/

#include <xo/randomgen/xoshiro256.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <xo/indentlog/print/vector.hpp>
#include <catch2/catch.hpp>
#include <algorithm>
#include <map>
#include <unordered_set>
#include <vector>

namespace utest {
    struct Util {
        /* generate vector with integers [0.. n-1] */
        static std::vector<std::uint32_t> vector_upto(std::uint32_t n) {
            std::vector<std::uint32_t> u(n);
            for (std::uint32_t i = 0; i < n; ++i)
                u[i] = i;

            return u;
        } /*vector_upto*/

        static std::map<std::uint32_t, std::uint32_t>
        map_upto(std::uint32_t n)
        {
            std::map<std::uint32_t, std::uint32_t> m;
            for(std::uint32_t i=0; i<n; ++i) {
                m[i] = i;
            }

            return m;
        } /*map_upto*/

        /* generate random permutation of integers [0.. n-1] */
        static std::vector<uint32_t>
        random_permutation(uint32_t n, xo::rng::xoshiro256ss *p_rgen) {
            /* vector [0 .. n-1] */
            std::vector<uint32_t> u = vector_upto(n);

            /* shuffle to get unpredictable permutation */
            std::shuffle(u.begin(), u.end(), *p_rgen);

            return u;
        } /*random_permutation*/
    }; /*Util*/

    // TODO: move REQUIRE_OR_CAPTURE(), REQUIRE_ORFAIL() to new subsystem xo-utestutil

/* note: trivial REQUIRE() call in else branch bc we still want
 *       catch2 to count assertions when verification succeeds
 */
#  define REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr) \
    if (catch_flag) {                                  \
        REQUIRE((expr));                               \
    } else {                                           \
        REQUIRE(true);                                 \
        ok_flag &= (expr);                             \
    }

#  define REQUIRE_ORFAIL(ok_flag, catch_flag, expr)    \
    REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr);      \
    if (!ok_flag)                                      \
        return ok_flag

    /** UtestTools
     **/
    struct UtestTools {
        /** bimodal may run twice:
         *  - first mode is silent, only determines success or failure.
         *  - second mode skipped when first mode succeeds.
         *    when first mode fails, second mode runs noisily with debug logging enabled
         *
         *  goal is to get detailed information from failing test;
         *  more detailed than feasible from catch2 INFO()
         *
         *  test function should use REQUIRE_ORCAPTURE() / REQUIRE_ORFAIL().
         *  It should *not* use REQUIRE() or CHECK().
         **/
        static inline bool bimodal_test(std::string test_name,
                                        std::function<bool (bool dbg_flag, std::uint32_t n)> test_fn,
                                        std::uint32_t n)
        {
            bool ok_flag = false;

            for (std::uint32_t attention = 0; !ok_flag && (attention < 2); ++attention) {
                bool debug_flag = (attention == 1);

                xo::scope log(XO_DEBUG2(debug_flag, test_name));

                ok_flag = test_fn(debug_flag, n);
            }

            return ok_flag;
        }
    };

    /* compare xo-ordinaltree/utest/random_tree_ops.hpp */
    template <typename HashMap>
    struct HashMapUtil : public Util {
#ifdef NOT_YET
        static bool
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

        static bool
        random_inserts(const std::vector<typename HashMap::key_type> & keys,
                       bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       HashMap * p_map)
        {
            using xo::xtag;

            bool ok_flag = true;

            xo::scope log(XO_DEBUG(catch_flag), xtag("n-keys", keys.size()));

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
        static bool
        random_inserts(std::uint32_t lo,
                       std::uint32_t hi,
                       std::uint32_t k,
                       bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       HashMap * p_map)
        {
            // TODO: rewrite in terms of 'random_inserts with explicit vector'.

            using xo::xtag;

            bool ok_flag = true;

            xo::scope log(XO_DEBUG(catch_flag), xtag("lo", lo), xtag("hi", hi), xtag("k", k));

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

        static bool
        random_inserts(std::uint32_t n,
                       bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       HashMap * p_map)
        {
            return random_inserts(0, n, 1, catch_flag, p_rgen, p_map);
        }

#ifdef NOT_YET
        /* do n random removes (taken from *p_rgen) from *p_rbtree;
         * assumes *p_rbtree has keys [0 .. n-1] where n=p_rbtreẹsize
         */
        static bool
        random_removes(bool catch_flag, // dbg_flag
                       xo::rng::xoshiro256ss * p_rgen,
                       Tree * p_map)
        {
            using xo::scope;
            using xo::xtag;

            bool ok_flag = true;

            xo::scope log(XO_DEBUG(catch_flag));

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
        static bool
        random_lookups(uint32_t dvalue,
                       bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       HashMap & map)
        {
            using xo::scope;
            using xo::xtag;

            xo::scope log(XO_DEBUG(catch_flag));

            /* -> false if/when verification fails */
            bool ok_flag = true;

            REQUIRE_ORFAIL(ok_flag, catch_flag, map.verify_ok());

            size_t n = map.size();
            std::vector<std::uint32_t> u
                = random_permutation(n, p_rgen);

            /* lookup keys in permutation order */
            std::uint32_t i = 1;
            for (std::uint32_t x : u) {
                INFO(tostr(xtag("i", i), xtag("n", n), xtag("x", x)));

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
        static bool
        check_forward_iterator(uint32_t dvalue,
                               bool catch_flag,
                               HashMap & map)
        {
            using xo::scope;
            using xo::xtag;

            /* -> flase if/when verification fails */
            bool ok_flag = true;

            std::size_t const n = map.size();

            scope log(XO_DEBUG(catch_flag));

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
        static bool
        check_backward_iterator(uint32_t dvalue,
                                bool catch_flag,
                                HashMap & map)
        {
            catch_flag=true;

            using xo::scope;
            using xo::xtag;

            /* -> flase if/when verification fails */
            bool ok_flag = true;

            std::size_t const n = map.size();

            scope log(XO_DEBUG(catch_flag));

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
        static bool
        check_bidirectional_iterator(uint32_t dvalue,
                                     bool catch_flag,
                                     Tree const & tree)
        {
            using xo::scope;
            using xo::xtag;

            /* -> false if/when verification fails */
            bool ok_flag = true;

            std::size_t const n = tree.size();

            xo::scope log(XO_DEBUG(catch_flag));

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
        static bool
        random_updates(uint32_t dvalue,
                       bool catch_flag,
                       Tree * p_rbtree,
                       xo::rng::xoshiro256ss * p_rgen)
        {
            using xo::scope;
            using xo::xtag;

            scope log(XO_DEBUG(catch_flag));

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
