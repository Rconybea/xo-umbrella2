/** @file RedBlackTree-gc.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "random_tree_ops.hpp"
#include "xo/ordinaltree/RedBlackTree.hpp"
#include "xo/ordinaltree/rbtree/SumReduce.hpp"
#include "xo/alloc/GC.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::gc::GC;
    using xo::tree::RedBlackTree;
    using xo::tree::SumReduce;

    using utest::TreeUtil;

    namespace ut {

        namespace {
            struct Testcase_RbTree {
                Testcase_RbTree(std::size_t nz,
                                std::size_t tz,
                                std::size_t ngct,
                                std::size_t tgct,
                                bool do_extra_gc) : nursery_z_{nz},
                                                    tenured_z_{tz},
                                                    incr_gc_threshold_{ngct},
                                                    full_gc_threshold_{tgct},
                                                    do_extra_gc_{do_extra_gc} {}


                std::size_t nursery_z_;
                std::size_t tenured_z_;
                std::size_t incr_gc_threshold_;
                std::size_t full_gc_threshold_;
                bool do_extra_gc_;
            };

            std::vector<Testcase_RbTree>
            s_testcase_v = {
                Testcase_RbTree(1024, 4096, 512, 512, false),
                Testcase_RbTree(1024, 4096, 512, 512, true),
            };
        }

        /* test that we can allocate RbTree::RbNode instances,
         * using gc allocator
         */
        TEST_CASE("rbnode-gc-1", "[gc][redblacktree]")
        {
            using RbTree = RedBlackTree<int,
                                        double,
                                        SumReduce<double>,
                                        xo::gc::allocator<std::pair<const int, double>>>;

            constexpr bool c_debug_flag = false;

            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const Testcase_RbTree & tc = s_testcase_v[i_tc];

                std::uint64_t seed = 8813374093428528487ULL;
                auto rgen = xo::rng::xoshiro256ss(seed);

                for (std::uint32_t n=0; n<1; ++n) {
                    scope log(XO_DEBUG2(c_debug_flag, "rbtree-gc-1"));

                    up<GC> gc = GC::make(
                                         {
                                             .initial_nursery_z_ = tc.nursery_z_,
                                             .initial_tenured_z_ = tc.tenured_z_,
                                             .incr_gc_threshold_ = tc.incr_gc_threshold_,
                                             .full_gc_threshold_ = tc.full_gc_threshold_,
                                             .debug_flag_ = c_debug_flag
                                         }
                                         );

                    xo::gc::allocator<RbTree::node_type> allocator(gc.get());

                    /* 1. verify that tree node can be constructed.
                     *    if it can't be constructed, the immediately-following construct
                     *    will fail in a non-transparent way.
                     */
                    RbTree::RbNode test_node;

                    RbTree::RbNode * test_node_ptr = new (MMPtr(gc.get())) RbTree::RbNode();
                    REQUIRE(test_node_ptr);

                    if (false) {
                        // this will compile, but can't will not run unless GC in progress
                        IObject * copy_ptr = test_node_ptr->_shallow_copy(gc.get());
                    }

                    /* 2. verify that tree node can be constructed via
                     *    allocator traits.
                     *
                     *    Reminder: {} expressions won't deduce template arguments
                     */
                    RbTree::node_allocator_traits::construct(allocator,
                                                             &test_node,
                                                             RbTree::RbNode::value_type{0, 0.0},
                                                             RbTree::RbNode::rvpair_type{0.0, 0.0});

                    /* 3. verify that RbNode::make_leaf() runs */
                    RbTree::RbNode * test2_node_ptr
                        = RbTree::RbNode::make_leaf(allocator,
                                                    RbTree::RbNode::value_type{0, 0.0},
                                                    0.0);
                    REQUIRE(test2_node_ptr);
                }
            }
        }

        /* test RbTree with gc allocator.
         *
         * do lots of GC-requesting, to create old->new cross-generational pointers
         */
        TEST_CASE("rbtree-gc-1", "[gc][redblacktree]")
        {
            using RbTree = RedBlackTree<int,
                                        double,
                                        SumReduce<double>,
                                        xo::gc::allocator<std::pair<const int, double>>>;

            constexpr bool c_debug_flag = false;

            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const Testcase_RbTree & tc = s_testcase_v[i_tc];

                std::uint64_t seed = 8813374093428528487ULL;
                auto rgen = xo::rng::xoshiro256ss(seed);

                for (std::uint32_t n=0; n<=1024;) {
                    bool ok_flag = false;

                    for (std::uint32_t attention = 0; !ok_flag && (attention < 2); ++attention) {
                        bool debug_flag = c_debug_flag || (attention == 1);

                        scope log(XO_DEBUG2(debug_flag, "rbtree-gc-1"), xtag("i_tc", i_tc), xtag("n", n));

                        INFO(tostr(xtag("i_tc", i_tc), xtag("n", n)));

                        ok_flag = true; // unless contradicted below

                        up<GC> gc = GC::make(
                                             {
                                                 .initial_nursery_z_ = tc.nursery_z_,
                                                 .initial_tenured_z_ = tc.tenured_z_,
                                                 .incr_gc_threshold_ = tc.incr_gc_threshold_,
                                                 .full_gc_threshold_ = tc.full_gc_threshold_,
                                                 .debug_flag_ = debug_flag
                                             }
                                             );
                        REQUIRE(gc.get());
                        gc->disable_gc();

                        REQUIRE(gc->native_gc_statistics().n_gc() == 0);

                        xo::gc::allocator<RbTree> allocator(gc.get());

                        gp<RbTree> rbtree = RbTree::make(allocator, c_debug_flag);

                        gc->add_gc_root_dwim(&rbtree);

                        REQUIRE(rbtree.get() != nullptr);
                        REQUIRE(rbtree->verify_ok(debug_flag));

                        if (tc.do_extra_gc_) {
                            REQUIRE(gc->gc_in_progress() == false);
                            gc->request_gc(gc::generation::nursery);
                            REQUIRE(gc->is_gc_pending());
                            REQUIRE(gc->enable_gc_once());
                            REQUIRE(gc->gc_in_progress() == false);
                        }

                        REQUIRE(rbtree->verify_ok(debug_flag));

                        {
                            INFO("insert phase A - random_inserts(0, n, 2, ..)");

                            /* insert even integers in [0, n), in random order **/
                            ok_flag &= TreeUtil<RbTree>::random_inserts(0, n, 2,
                                                                        debug_flag,
                                                                        &rgen,
                                                                        rbtree.get());

                            if (tc.do_extra_gc_) {
                                REQUIRE(gc->gc_in_progress() == false);
                                gc->request_gc(gc::generation::nursery);
                                REQUIRE(gc->is_gc_pending());
                                REQUIRE(gc->enable_gc_once());
                                REQUIRE(gc->gc_in_progress() == false);
                            }

                            REQUIRE(rbtree->verify_ok(debug_flag));
                            
                        }

                        if (n > 0) {
                            INFO("insert phase B - random_inserts(1, n+1, ..)");

                            /* insert odd integers in [1, n+1), in random order **/
                            ok_flag &= TreeUtil<RbTree>::random_inserts(1, n+1, 2,
                                                                        debug_flag,
                                                                        &rgen,
                                                                        rbtree.get());

                            if (tc.do_extra_gc_) {
                                REQUIRE(gc->gc_in_progress() == false);
                                gc->request_gc(gc::generation::nursery);
                                REQUIRE(gc->is_gc_pending());
                                REQUIRE(gc->enable_gc_once());
                                REQUIRE(gc->gc_in_progress() == false);
                            }

                            REQUIRE(rbtree->verify_ok(debug_flag));
                        }

                        /* check iterator traverses [0..n-1] in both directions */
                        ok_flag &= TreeUtil<RbTree>::check_ordinal_lookup(0 /*dvalue*/,
                                                                          debug_flag,
                                                                          *rbtree);

                        /* verify end-to-end iteration */
                        ok_flag &= TreeUtil<RbTree>::check_bidirectional_iterator(0,
                                                                                  debug_flag,
                                                                                  *rbtree);

                        /* check reduced sums for each cut */
                        ok_flag &= TreeUtil<RbTree>::check_reduced_sum(0,
                                                                       debug_flag,
                                                                       *rbtree);

                        /* verify read-only variant of operator[] */
                        ok_flag &= TreeUtil<RbTree>::random_lookups(debug_flag,
                                                                    *rbtree,
                                                                    &rgen);

                        /* re-verify lookup on (better-be-monotonic) reduced sums
                         * remove doubt that operator[] changed something.
                         */
                        ok_flag &= TreeUtil<RbTree>::check_ordinal_lookup(0 /*dvalue*/,
                                                                          debug_flag,
                                                                          *rbtree);

                        /* re-verify end-to-end iteration, so we can say we did */
                        ok_flag &= TreeUtil<RbTree>::check_bidirectional_iterator(0,
                                                                                  debug_flag,
                                                                                  *rbtree);

                        /* make random updates, along with basic consistency checks */
                        ok_flag &= TreeUtil<RbTree>::random_updates(10000,
                                                                    debug_flag,
                                                                    rbtree.get(),
                                                                    &rgen);
                        REQUIRE(rbtree->verify_ok(debug_flag));

                        
                        if (tc.do_extra_gc_) {
                            REQUIRE(gc->gc_in_progress() == false);
                            gc->request_gc(gc::generation::nursery);
                            REQUIRE(gc->is_gc_pending());
                            REQUIRE(gc->enable_gc_once());
                            REQUIRE(gc->gc_in_progress() == false);
                        }

                        REQUIRE(rbtree->verify_ok(debug_flag));

                        /* verify that updates changed tree contents in expected way */
                        ok_flag &= TreeUtil<RbTree>::check_ordinal_lookup(10000 /*dvalue*/,
                                                                          debug_flag,
                                                                          *rbtree);

                        /* verify end-to-end iteration */
                        ok_flag &= TreeUtil<RbTree>::check_bidirectional_iterator(10000,
                                                                                  debug_flag,
                                                                                  *rbtree);

                        /* check reduced sums for each cut */
                        ok_flag &= TreeUtil<RbTree>::check_reduced_sum(10000,
                                                                       debug_flag,
                                                                       *rbtree);

                        /* verify behavior of read/write variant of operator[] */
                        ok_flag &= TreeUtil<RbTree>::random_removes(debug_flag,
                                                                    &rgen,
                                                                    rbtree.get());

                        if (tc.do_extra_gc_) {
                            REQUIRE(gc->gc_in_progress() == false);
                            gc->request_gc(gc::generation::nursery);
                            REQUIRE(gc->is_gc_pending());
                            REQUIRE(gc->enable_gc_once());
                            REQUIRE(gc->gc_in_progress() == false);
                        }

                        /* verify iteration one more time --
                         * remove doubt w.r.t.
                         */
                        REQUIRE(rbtree->verify_ok(debug_flag));
                    }

                    if (n == 0)
                        n = 1;
                    else
                        n = 2*n;
                }
            }

        }

    } /*namespace ut*/
} /*namesapce xo*/

/* end RedBlackTree-gc.test.cpp */
