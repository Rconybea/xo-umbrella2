/* @file redblacktree.cpp */

#include "random_tree_ops.hpp"
#include "xo/ordinaltree/RedBlackTree.hpp"
#include <map>

namespace {
    using xo::tree::RedBlackTree;
    using xo::tree::SumReduce;
    using xo::tree::OrdinalReduce;
    using xo::tree::NullReduce;
    using xo::rng::xoshiro256ss;

    using utest::Util;
    using utest::TreeUtil;

    using xo::scope;
    using xo::scope_setup;
    using xo::xtag;

    //using RbTree = RedBlackTree<int, double, OrdinalReduce<double>>;
    using RbTree = RedBlackTree<int, double, SumReduce<double>>;

#ifdef OBSOLETE
  /* Require:
   * - rbtree has keys [0..n-1] where n=rbtree.size(),
   * - rbtree value at key k is dvalue+10*k
   */
  void
  check_ordinal_lookup(uint32_t dvalue,
               RbTree const & rbtree)
  {
    size_t const n = rbtree.size();
    size_t i = 0;

    for(size_t i=0; i<n; ++i) {
      RbTree::const_iterator ix = rbtree.find_ith(i);

      REQUIRE(ix != rbtree.end());
      REQUIRE(ix->first == i);
    }
  } /*check_ordinal_lookup*/
#endif

  /* check that RedBlackTree<>::find_sum_glb() works as advertised.
   *
   * partial sums of v[j] for j<=i will be:
   *
   *        (i+1) . i
   *   10 . ---------  + ((i+1) . dvalue)
   *            2
   *
   *  = (i+1).(5.i + dvalue)
   *
   * Require:
   * - rbtree has keys [0..n-1],  where n=rbtree.size()
   * - rbtree value at key k is dvalue+10*k
   */
  void
  check_reduced_sum(uint32_t dvalue,
            RbTree const & rbtree)
  {
    size_t const n = rbtree.size();

    for(size_t i = 0; i < n; ++i) {
      /* compute reduction up to key=i */
      double reduced_upto
    = rbtree.reduce_lub(i /*key*/,
                true /*is_closed*/);

      double reduced = (i+1) * (5*i + dvalue);

      INFO(tostr(xtag("i", i), xtag("n", n),
         xtag("tree.reduced_upto", reduced_upto),
         xtag("reduced", reduced),
         xtag("dvalue", dvalue)));

      auto glb_ix = rbtree.cfind_sum_glb(reduced);

      REQUIRE(reduced_upto == reduced);

      REQUIRE(glb_ix.is_dereferenceable());
      /* glb_ix is truth-y */
      REQUIRE(glb_ix);

      REQUIRE(glb_ix->first == i);
    }
  } /*check_reduced_sum*/

#ifdef OBSOLETE
  /* Require:
   * - *p_rbtree has keys [0..n-1],  where n=rbtree.size()
   * - for each key k,  associated value is 10*k
   */
  void
  random_lookups(RbTree const & rbtree,
         xoshiro256ss * p_rgen)
  {
    REQUIRE(rbtree.verify_ok());

    size_t n = rbtree.size();
    std::vector<uint32_t> u
        = Util::random_permutation(n, p_rgen);

    /* lookup keys in permutation order */
    uint32_t i = 1;
    for (uint32_t x : u) {
      INFO(tostr(xtag("i", i), xtag("n", n), xtag("x", x)));

      REQUIRE(rbtree[x] == x*10);
      REQUIRE(rbtree.verify_ok());
      REQUIRE(rbtree.size() == n);
      ++i;
    }

    REQUIRE(rbtree.size() == n);
  } /*random_lookups*/
#endif

  /* Require:
   * - *p_rbtree has keys [0..n-1],  where n=rbtree.size()
   * - for each key k,  associated value is 10*k
   *
   * Promise:
   * - for each key k,  associated value is dvalue + 10*k
   */
  void
  random_updates(uint32_t dvalue,
                 RbTree * p_rbtree,
                 xoshiro256ss * p_rgen)
  {
    REQUIRE(p_rbtree->verify_ok());

    std::size_t n = p_rbtree->size();
    std::vector<uint32_t> u
        = Util::random_permutation(n, p_rgen);

    /* update key/value pairs in permutation order */
    uint32_t i = 1;
    for (uint32_t x : u) {
      REQUIRE((*p_rbtree)[x] == x*10);

      (*p_rbtree)[x] = dvalue + 10*x;

      REQUIRE((*p_rbtree)[x] == dvalue + 10*x);
      REQUIRE(p_rbtree->verify_ok());
      /* assignment to existing key does not change tree size */
      REQUIRE(p_rbtree->size() == n);
      ++i;
    }

    REQUIRE(p_rbtree->size() == n);
  } /*random_updates_1*/

    TEST_CASE("rbtree", "[redblacktree]") {
        constexpr bool c_debug_flag = false;
        RbTree rbtree{RbTree::allocator_type{}, c_debug_flag};

        std::uint64_t seed = 14950349842636922572UL;
        /* can reseed from /dev/urandom with: */
        //arc4random_buf(&seed, sizeof(seed));

        auto rgen = xo::rng::xoshiro256ss(seed);

        /* perform a series of tests with increasing scale */
        for(std::uint32_t n=0; n<=1024; ) {
            bool ok_flag = false;

            for (std::uint32_t attention = 0; !ok_flag && (attention < 2); ++attention) {
                /* attention=0:
                 *   - no logging
                 *   - detect assertion failures,  but don't report them to catch2
                 * attention=1:
                 *   - only runs if failure detected with attention=0
                 *   - full logging
                 *   - report to catch
                 */

                bool debug_flag = (attention == 1);

                scope log(XO_DEBUG2(debug_flag, "rbtree"));
                log && log(xtag("size", n));

                ok_flag = true;

                if (n == 0) {
                    /* check iteration on empty tree */
                    ok_flag &= TreeUtil<RbTree>::check_bidirectional_iterator(0 /*dvalue - not used*/,
                                                                              debug_flag,
                                                                              rbtree);
                } else {
                    /* insert [0..n-1] in random order */
                    ok_flag &= TreeUtil<RbTree>::random_inserts(n, debug_flag, &rgen, &rbtree);

                    /* TODO: generalize remaining helpers;  share with bplustree unit test */

                    /* check iterator traverses [0..n-1] in both directions (using ++ and --) */
                    ok_flag &= TreeUtil<RbTree>::check_ordinal_lookup(0 /*dvalue*/,
                                                                      debug_flag,
                                                                      rbtree);
                    /* verify end-to-end iteration */
                    ok_flag &= TreeUtil<RbTree>::check_bidirectional_iterator(0,
                                                                              debug_flag,
                                                                              rbtree);
                    /* verify behavior of .reduce_lub(), .find_sum_glb() */
                    check_reduced_sum(0, rbtree);
                    /* verify behavior of read-only variant of operator[] */
                    ok_flag &= TreeUtil<RbTree>::random_lookups(debug_flag,
                                                                rbtree,
                                                                &rgen);

                    /* verify that lookups didn't somehow disturb tree contents */
                    ok_flag &= TreeUtil<RbTree>::check_ordinal_lookup(0 /*dvalue*/,
                                                                      debug_flag,
                                                                      rbtree);

                    ok_flag &= TreeUtil<RbTree>::check_bidirectional_iterator(0,
                                                                              debug_flag,
                                                                              rbtree);
                    /* verify update via read/write operator[] */
                    random_updates(10000, &rbtree, &rgen);

                    /* verify that updates changed tree contents in expected way */
                    ok_flag &= TreeUtil<RbTree>::check_ordinal_lookup(10000 /*dvalue*/,
                                                                      debug_flag,
                                                                      rbtree);

                    /* verify end-to-end iteration */
                    ok_flag &= TreeUtil<RbTree>::check_bidirectional_iterator(10000,
                                                                              debug_flag,
                                                                              rbtree);
                    /* verify behavior of .reduce_lub(), .find_sum_glb() */
                    check_reduced_sum(10000, rbtree);
                    /* verify behavior of read/write variant of operator[] */
                    ok_flag &= TreeUtil<RbTree>::random_removes(debug_flag, &rgen, &rbtree);
                }

                log.end_scope();
            }

            if (n == 0)
                n = 1;
            else
                n = 2*n;
        }
    } /*TEST_CASE(rbtree)*/
} /*namespace*/

/* end redblacktree.cpp */
