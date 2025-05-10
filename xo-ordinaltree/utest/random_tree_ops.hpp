/* @file random_tree_ops.hpp **/

#include "xo/randomgen/xoshiro256.hpp"
#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/tag.hpp"
#include "xo/indentlog/print/vector.hpp"
#include "catch2/catch.hpp"
#include <algorithm>
#include <map>
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


    template <typename Tree>
    struct TreeUtil : public Util {
        static bool
        test_clear(bool catch_flag,
                   Tree * p_tree)
        {
            bool ok_flag = true;

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->verify_ok());

            p_tree->clear();

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->verify_ok(catch_flag));
            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->empty());
            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->size() == 0);

            return ok_flag;
        } /*test_clear*/

        /* do n random inserts (taken from *p_rgen) into *p_rbtreẹ
         * inserted keys will be distinct values in [0, .., n-1]
         */
        static bool
        random_inserts(std::uint32_t n,
                       bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       Tree * p_tree)
        {
            using xo::xtag;

            bool ok_flag = true;

            xo::scope log(XO_DEBUG(catch_flag));

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->verify_ok());

            /* n keys 0..n-1 */
            std::vector<std::uint32_t> u(n);
            for(std::uint32_t i=0; i<n; ++i)
                u[i] = i;

            /* shuffle to get unpredictable insert order */
            std::shuffle(u.begin(), u.end(), *p_rgen);

            /* insert keys according to permutation u */
            uint32_t i = 1;
            for(uint32_t x : u) {
                log && log(xtag("i", i), xtag("n", n), xtag("key", x));
                /* .first:  iterator @ insert position
                 * .second: true if insert occurred (ịẹ tree size incremented)
                 */
                auto insert_result = p_tree->insert(typename Tree::value_type(x, 10 * x));

                REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->verify_ok(catch_flag));

                REQUIRE_ORFAIL(ok_flag, catch_flag, insert_result.second);

                /* verify: iterator returned by Treẹinsert(),  refers to inserted key,value pair */
                log && log(xtag("iter.node", insert_result.first.node()));
                REQUIRE_ORFAIL(ok_flag, catch_flag, insert_result.first->first == x);
                REQUIRE_ORFAIL(ok_flag, catch_flag, insert_result.first->second == 10 * x);

                ++i;
            }

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->size() == n);

            return ok_flag;
        } /*random_inserts*/

        /* do n random removes (taken from *p_rgen) from *p_rbtree;
         * assumes *p_rbtree has keys [0 .. n-1] where n=p_rbtreẹsize
         */
        static bool
        random_removes(bool catch_flag,
                       xo::rng::xoshiro256ss * p_rgen,
                       Tree * p_tree)
        {
            using xo::scope;
            using xo::xtag;

            bool ok_flag = true;

            xo::scope log(XO_DEBUG(catch_flag));

            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->verify_ok(catch_flag));

            uint32_t n = p_tree->size();

            /* random permutation of keys in *p_tree */
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

                p_tree->erase(x);
                // rbtreẹdisplay();
                REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->size() == n-i);
                /* amongst other things,  this guarantees that keys in *p_tree
                 * appear in increasing order
                 */
                REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->verify_ok(catch_flag));

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
                p_tree->visit_inorder(visitor_fn);
#endif
                ++i;
            }

            REQUIRE_ORFAIL(ok_flag, catch_flag, m.empty());
            REQUIRE_ORFAIL(ok_flag, catch_flag, p_tree->size() == 0);

            log.end_scope();

            return ok_flag;
        } /*random_removes*/

        /* Require:
         * - tree has keys [0..n-1],  where n=treẹsize()
         * - for each key k,  associated value is 10*k
         */
        static bool
        random_lookups(bool catch_flag,
                       Tree const & tree,
                       xo::rng::xoshiro256ss * p_rgen)
        {
            using xo::scope;
            using xo::xtag;

            xo::scope log(XO_DEBUG(catch_flag));

            /* -> false if/when verification fails */
            bool ok_flag = true;

            REQUIRE_ORFAIL(ok_flag, catch_flag, tree.verify_ok(catch_flag));

            size_t n = tree.size();
            std::vector<std::uint32_t> u
                = random_permutation(n, p_rgen);

            /* lookup keys in permutation order */
            std::uint32_t i = 1;
            for (std::uint32_t x : u) {
                INFO(tostr(xtag("i", i), xtag("n", n), xtag("x", x)));

                REQUIRE_ORFAIL(ok_flag, catch_flag, tree[x] == x*10);
                REQUIRE_ORFAIL(ok_flag, catch_flag, tree.verify_ok(catch_flag));
                REQUIRE_ORFAIL(ok_flag, catch_flag, tree.size() == n);

                /* also test treẹfind() */
                auto find_ix = tree.find(x);

                REQUIRE_ORFAIL(ok_flag, catch_flag, find_ix != tree.end());
                REQUIRE_ORFAIL(ok_flag, catch_flag, find_ix->first == x);
                REQUIRE_ORFAIL(ok_flag, catch_flag, find_ix->second == x*10);

                ++i;
            }

            REQUIRE_ORFAIL(ok_flag, catch_flag, tree.size() == n);

            log.end_scope();

            return ok_flag;
        } /*random_lookups*/

        /* Require:
         * - tree has keys [0..n-1],  where n=treẹsize()
         * - tree value at key k is dvalue+10*k
         */
        static bool
        check_ordinal_lookup(std::uint32_t dvalue,
                             bool catch_flag,
                             Tree const & tree)
        {
            using xo::scope;
            using xo::xtag;

            /* -> false if/when verification fails */
            bool ok_flag = true;

            xo::scope log(XO_DEBUG(catch_flag));

            std::size_t const n = tree.size();
            std::size_t i = 0;

            log && log("tree with size n", xtag("n", n));

            for (std::size_t i=0; i<n; ++i) {
                typename Tree::const_iterator ix;

                try {
                    ix = tree.find_ith(i);  /* find_ith() may throw if broken */
                } catch(...) {
                    if (catch_flag)
                        throw;
                }

                REQUIRE_ORFAIL(ok_flag, catch_flag, ix.is_dereferenceable());
                REQUIRE_ORFAIL(ok_flag, catch_flag, (ix != tree.end()));
                REQUIRE_ORFAIL(ok_flag, catch_flag, (ix->first == i));
                REQUIRE_ORFAIL(ok_flag, catch_flag, (ix->second == 10*i + dvalue));
            }

            log.end_scope();

            return ok_flag;
        } /*check_ordinal_lookup*/

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
    }; /*TreeUtil*/
} /*namespace utest*/

/* end random_tree_ops.hpp */
