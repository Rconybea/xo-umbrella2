/** @file RedBlackTree-gc.test.cpp
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
                                std::size_t tgct) : nursery_z_{nz},
                                                    tenured_z_{tz},
                                                    incr_gc_threshold_{ngct},
                                                    full_gc_threshold_{tgct} {}
                                                                      

                std::size_t nursery_z_;
                std::size_t tenured_z_;
                std::size_t incr_gc_threshold_;
                std::size_t full_gc_threshold_;
            };

            std::vector<Testcase_RbTree>
            s_testcase_v = {
                Testcase_RbTree(1024, 4096, 512, 512),
            };
        }

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

                for (std::uint32_t n=0; n<1; ++n) {
                    bool ok_flag = false;

                    for (std::uint32_t attention = 0; !ok_flag && (attention < 2); ++attention) {
                        bool debug_flag = c_debug_flag || (attention == 1);

                        scope log(XO_DEBUG2(debug_flag, "rbtree-gc-1"));

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

                        xo::gc::allocator<RbTree::node_type> allocator(gc.get());

                        /* 1. verify that tree node can be constructed.
                         *    if it can't be constructed, the immediately-following construct
                         *    will fail in a non-transparent way.
                         */
                        RbTree::RbNode test_node;

#ifdef NOT_YET
                        /* 2. verify that tree node can be constructed via
                         *    allocator traits
                         */
                        RbTree::node_allocator_traits::construct(allocator,
                                                                 &test_node,
                                                                 {0, 0.0},
                                                                 {0.0, 0.0});

                        RbTree rbtree(allocator, c_debug_flag);

                        /* insert [0..n-1] in random order **/
                        ok_flag &= TreeUtil<RbTree>::random_inserts(n, debug_flag, &rgen, &rbtree);
#endif
                    }
                }
            }

        }

    } /*namespace ut*/
} /*namesapce xo*/

/* end RedBlackTree-gc.test.cpp */

