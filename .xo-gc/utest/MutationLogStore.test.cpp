/** @file MutationLogStore.test.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GcosTestutil.hpp"
#include "MlsTestutil.hpp"
#include <xo/object2/List.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/gc/GCObjectStore.hpp>
#include <xo/gc/GCObjectStoreVisitor.hpp>
#include <xo/gc/MutationLogStore.hpp>
#include <xo/gc/X1VerifyStats.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/randomgen/xoshiro256.hpp>
#include <xo/randomgen/random_seed.hpp>
#include <catch2/catch.hpp>
#include <unistd.h> // for ::getpagesize()

namespace ut {
    using xo::scm::DList;
    using xo::scm::DBoolean;
    using xo::scm::DInteger;
    using xo::mm::MutationLogStore;
    using xo::mm::MutationLogConfig;
    using xo::mm::MutationLog;
    using xo::mm::MutationLogEntry;
    using xo::mm::GCObjectStore;
    using xo::mm::GCObjectStoreConfig;
    using xo::mm::DGCObjectStoreVisitor;
    using xo::mm::Role;
    using xo::mm::Generation;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::mm::X1VerifyStats;
    using xo::rng::xoshiro256ss;
    using xo::rng::random_seed;
    using xo::reflect::typeseq;
    using xo::xtag;
    using xo::scope;

    namespace {
        struct Testcase {
            explicit Testcase(uint32_t n_gen,
                              uint32_t n_survive,
                              size_t gc_z,
                              uint32_t type_z,
                              bool do_type_registration,
                              TestSequence test_seq,
                              uint32_t mlog_z,
                              bool mlog_enabled_flag,
                              TestGraphType obj_graph_type,
                              uint32_t n_gc_loop,
                              uint32_t n_i0_test_obj,
                              uint32_t n_i0_test_assign,
                              uint32_t n_i1_test_obj,
                              uint32_t n_i1_test_assign,
                              bool debug_flag)
            : n_gen_{n_gen},
              n_survive_{n_survive},
              gc_size_{gc_z},
              object_type_z_{type_z},
              do_type_registration_{do_type_registration},
              mutation_log_z_{mlog_z},
              mlog_enabled_flag_{mlog_enabled_flag},
              test_seq_{test_seq},
              obj_graph_type_{obj_graph_type},
              n_gc_loop_{n_gc_loop},
              n_i0_test_obj_{n_i0_test_obj},
              n_i0_test_assign_{n_i0_test_assign},
              n_i1_test_obj_{n_i1_test_obj},
              n_i1_test_assign_{n_i1_test_assign},
              debug_flag_{debug_flag}
            {}

            bool sanitize_flag() const noexcept { return true; }

            /** number of generations in gco store **/
            uint32_t n_gen_ = 0;
            /** object prommotes on surviving this many gc cycles **/
            uint32_t n_survive_ = 0;
            /** size of each generation's half-space in bytes **/
            size_t gc_size_ = 0;
            /** storage for object type in bytes **/
            uint32_t object_type_z_ = 0;
            /** if true register types for gc-aware types used in unit test **/
            bool do_type_registration_ = false;
            /** storage for mutation log (mult by 3 x n_gen_) **/
            uint32_t mutation_log_z_ = 0;
            /** true if enabling mutation-log feature
             *  (load-bearing for incremental gc)
             **/
            bool mlog_enabled_flag_ = false;
            /** if non-null; run contents of cmd_seq_[i] on loop #i **/
            TestSequence test_seq_;
            /** object graph type **/
            TestGraphType obj_graph_type_ = TestGraphType::random;
            /** #of gc-like "move all the roots" phases to perform **/
            uint32_t n_gc_loop_ = 0;
            /** 2nd loop: #of cells in random object graph **/
            uint32_t n_i0_test_obj_ = 0;
            /** 2nd loop: #of random assignments to attempt **/
            uint32_t n_i0_test_assign_ = 0;
            /** 3rd+later loop: #of cells in random object graph **/
            uint32_t n_i1_test_obj_ = 0;
            /** 3rd+later loop: #of random assignments to attempt **/
            uint32_t n_i1_test_assign_ = 0;
            /** true to enable debug when attempting this test case **/
            bool debug_flag_ = false;
        };

        constexpr TestGraphType c_selfcycle = TestGraphType::selfcycle;
        constexpr TestGraphType c_random = TestGraphType::random;
        constexpr TestGraphType c_fixed = TestGraphType::fixed;

        using Cmd = Step::Cmd;

        static Step step_0[] = {
            {Cmd::make_bool, 0, 0}, // #f
            {Cmd::make_nil,  0, 0},  // #nil
            {Cmd::make_cons, 0, 1}, // cons(#f,#nil)
            {Cmd::sentinel,  0, 0},
        };

        static Phase phase_0[] = {
            //
            // lo   hi    mlog_new_z_[]
            //  v    v    v
            {   0,   3,   {0} },
            {  -1,  -1,   {0} },
        };

        static TestSequence seq_0 { step_0, phase_0 };

        // ----------------------------------------------------------------

        // seq1: side effect on head of cons cell.
        // But no mlog entry b/c all object ages are equal
        // -> no x-age pointers
        //
        static Step step_1[] = {
            {Cmd::make_bool,   0, 0}, // [0]: #f
            {Cmd::make_bool,   1, 0}, // [1]: #t
            {Cmd::make_nil,    0, 0}, // [2]: #nil
            {Cmd::make_cons,   0, 2}, // [3]: cons(#f,#nil)
            {Cmd::assign_head, 3, 1}, //      set-car(cons(#f,#nil),#t)
            {Cmd::sentinel,    0, 0},
        };

        static Phase phase_1[] = {
            //
            // lo   hi    mlog_new_z_[]
            //  v    v    v
            {   0,   5,   {0} },
            {  -1,  -1,   {0} },
        };

        static TestSequence seq_1 { step_1, phase_1 };

        // ----------------------------------------------------------------

        static Step step_2[] = {
            // ----- phase 0 -----
            {Cmd::make_bool,   0, 0}, // [0]: #f
            {Cmd::make_bool,   1, 0}, // [1]: #t
            {Cmd::make_nil,    0, 0}, // [2]: #nil
            {Cmd::make_cons,   0, 2}, // [3]: cons(#f,#nil)
            // ----- phase 1 -----
            {Cmd::make_bool,   1, 0}, // [4]: #t
            {Cmd::assign_head, 3, 4}, //      set-car(cons(#f,#nil),#t)
            // ----- phase 2 -----
            // ----- end -----
            {Cmd::sentinel,    0, 0},
        };

        static Phase phase_2[] = {
            //
            // lo   hi    mlog_new_z_[]
            //  v    v    v
            {   0,   4,   {0} },  // phase 0
            {   4,   6,   {1} },  // phase 1. set-car makes 1x xgen ptr from g1->g0
            {   6,   6,   {0} },  // phase 2. now both {src,dest} are in g1
            {  -1,  -1,   {0} },
        };

        static TestSequence seq_2 { step_2, phase_2 };

        // ----------------------------------------------------------------

        static Step step_3[] = {
            // ----- phase 0 -----
            {Cmd::make_bool,   0, 0}, // [0]: #f
            {Cmd::make_bool,   1, 0}, // [1]: #t
            {Cmd::make_nil,    0, 0}, // [2]: #nil
            {Cmd::make_cons,   0, 2}, // [3]: cons(#f,#nil)
            // ----- phase 1 -----
            {Cmd::make_bool,   1, 0}, // [4]: #t
            {Cmd::assign_head, 3, 4}, //      set-car(cons(#f,#nil),#t)
            // ----- phase 2 -----
            // ----- phase 3 -----
            // ----- end -----
            {Cmd::sentinel,    0, 0},
        };

        static Phase phase_3[] = {
            //
            // lo   hi    mlog_new_z_[]
            //  v    v    v
            {   0,   4,   {0} },  // phase 0
            {   4,   6,   {1} },  // phase 1. set-car makes 1x xage ptr
            {   6,   6,   {1} },  // phase 2. now src in g1, dest in g0
            {   6,   6,   {0} },  // phase 3. now dest in g1
            {  -1,  -1,   {0} },
        };

        static TestSequence seq_3 { step_3, phase_3 };

        // ----------------------------------------------------------------

        static Step step_4[] = {
            // ----- phase 0 -----
            {Cmd::make_bool,   0, 0}, // [0]: #f
            {Cmd::make_bool,   1, 0}, // [1]: #t
            {Cmd::make_nil,    0, 0}, // [2]: #nil
            {Cmd::make_cons,   0, 2}, // [3]: cons(#f,#nil)

            // 1st gc

            // ----- phase 1 -----
            {Cmd::make_bool,   1, 0}, // [4]: #t
            {Cmd::assign_head, 3, 4}, //      set-car([3],#t)

            // 2nd gc. [0]..[3] promote to g1
            // [4] in g0 so [3]->[4] requires mlog entry

            // ----- phase 2 -----
            {Cmd::make_bool,   0, 0}, // [5]: #f
            {Cmd::assign_head, 3, 5}, //      set-car([3],#f)

            // 3rd gc. [4] promotes to g1,
            // [5] in g0 so [3]->[5] requires mlog entry

            // ----- phase 3 -----
            // ----- phase 4 -----
            // ----- end -----
            {Cmd::sentinel,    0, 0},
        };

        static Phase phase_4[] = {
            //
            // lo   hi    mlog_new_z_[]
            //  v    v    v
            {   0,   4,   {0} },  // phase 0 gc
            {   4,   6,   {1} },  // phase 1 gc. set-car makes 1x xage ptr
            {   6,   8,   {2} },  // phase 2 gc. now src in g1, dest [4] in g0
            {   8,   8,   {1} },  // phase 3 gc. new dest [5] in g0
            {   8,   8,   {0} },  // phase 4 gc. now dest in g1
            {  -1,  -1,   {0} },
        };

        static TestSequence seq_4 { step_4, phase_4 };

        // ----------------------------------------------------------------

        static Step step_5[] = {
            // ----- phase 0 -----
            {Cmd::make_int,   99, 0}, // [0]: 99
            {Cmd::make_nil,    0, 0}, // [1]: #nil
            {Cmd::make_cons,   0, 1}, // [2]: cons([0],[1]) -> cons(99,#nil)

            // phase 0 gc (1st gc)

            // ----- phase 1 -----

            {Cmd::make_int,   15, 0}, // [3]: 15
            {Cmd::assign_head, 2, 3}, //      set-car([2],[3]) -> set-car([2],15)

            // phase 1 gc (2nd gc)
            // [1]..[2] promote to g1
            // [3] in g0 so [2]->[3] requires mlog entry

            // ----- phase 2 -----
            {Cmd::make_int,   24, 0}, // [4]: 33
            {Cmd::assign_head, 2, 4}, //      set-car([2],[4]) -> set-car([2],33)

            // phase 2 gc (3rd gc)

            // ----- phase 3 -----

            {Cmd::assign_root, 2, 0}, //      [2] = [0] = 99

            // phase 3 gc (4th gc)
            // o.g. o.g. [2] would be garbage, if we collected g1

            // ----- phase 4 -----

            // ----- end -----
            {Cmd::sentinel,    0, 0},
        };

        static Phase phase_5[] = {
            //
            // lo   hi    mlog_new_z_[]
            //  v    v    v
            {   0,   3,   {0} },  // phase 0 gc
            {   3,   5,   {1} },  // phase 1 gc. set-car makes 1x xage ptr
            {   5,   7,   {2} },  // phase 2 gc. now src in g1, dest [3] in g0
            {   7,   8,   {1} },  // phase 3 gc. new dest [4] in g0
            {   8,   8,   {0} },  // phase 4 gc. now dest [4] in g1
            {  -1,  -1,   {0} },
        };

        static TestSequence seq_5 { step_5, phase_5 };


        // ----------------------------------------------------------------

#      define seq_nil TestSequence{}
#      define nil nullptr
#      define T true
#      define F false

        static std::vector<Testcase> s_testcase_v = {
            /**
             *                                                                             debug_flag
             *                                                                    n_i1_test_assign  |
             *                                                                   n_i1_test_obj   |  |
             *                                                            n_i0_test_assign   |   |  |
             *                                                           n_i0_test_obj   |   |   |  |
             *                                                           n_gc_loop   |   |   |   |  |
             *                                                   obj_graph_type  |   |   |   |   |  |
             *                                   mlog_enabled_flag            |  |   |   |   |   |  |
             *                                   mutation_log_z  |            |  |   |   |   |   |  |
             *                                    cmd_seq     |  |            |  |   |   |   |   |  |
             *              do_type_registration        |     |  |            |  |   |   |   |   |  |
             *  n_survive       object_type_z  |        |     |  |            |  |   |   |   |   |  |
             *   n_gen  |    gc_size        |  |        |     |  |            |  |   |   |   |   |  |
             *       v  v          v        v  v        v     v  v            v  v   v   v   v   v  v
             **/
            Testcase(2, 4, 16 * 1024, 8 * 128, F, seq_nil,    0, F,    c_random, 1,  0,  0,  0,  0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, seq_nil,    0, F, c_selfcycle, 1,  1,  0,  0,  0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T,   seq_0,    0, F,     c_fixed, 1,  0,  0,  0,  0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T,   seq_1,    0, F,     c_fixed, 1,  0,  0,  0,  0, F),
            Testcase(2, 1, 16 * 1024, 8 * 128, T,   seq_2,  128, T,     c_fixed, 3,  0,  0,  0,  0, F),
            Testcase(2, 2, 16 * 1024, 8 * 128, T,   seq_3,  128, T,     c_fixed, 4,  0,  0,  0,  0, F),
            Testcase(2, 2, 16 * 1024, 8 * 128, T,   seq_4,  128, T,     c_fixed, 4,  0,  0,  0,  0, F),
            Testcase(2, 2, 16 * 1024, 8 * 128, T,   seq_5,  128, T,     c_fixed, 4,  0,  0,  0,  0, F),
        };

#      undef T
#      undef F

        /** Fixture for MutationLogStore-1 test.
         *  Compare similar but not identical fixture in GCObjectStore.test.cpp
         **/
        class MlsFixture {
        public:
            explicit MlsFixture(const Testcase &);

            /** configuration for @ref gcos_ **/
            GCObjectStoreConfig gcos_config_;
            /** configuration for @ref mls_ **/
            MutationLogConfig mls_config_;

            /** Parallel arena for reference
             *
             *  We will allocate parallel object model in this arena
             *  for reference; then compare with GCObjectStore behavior.
             *
             *  1. arena2 doesn't have any generation layer cake stuff.
             *     all objects are in one place
             *  2. arena2 doesn't have concept of installed types.
             *     It doesn't have or require any builtin ability to traverse an object model,
             *     storage recovery strategy is O(1) "clear the whole arena".
             **/
            DArena arena2_;

            /** statistics called by GCObjectStore.verify_ok() **/
            X1VerifyStats verify_stats_;
            /** holds objects in multiple generations.
             **/
            GCObjectStore gcos_;
            /**
             *  mutation log store tracks pointers
             *  from older objects to younger objects,
             *  which can only be created by mutation
             **/
            MutationLogStore mls_;
        };

        MlsFixture::MlsFixture(const Testcase & tc)
            : gcos_config_{(ArenaConfig()
                            .with_name("mlog-fixture-arena-name-notused")
                            .with_size(tc.gc_size_)
                            .with_store_header_flag(true)),
                            tc.n_gen_,
                            tc.n_survive_,
                            tc.object_type_z_,
                            tc.debug_flag_},
              mls_config_{tc.n_gen_,
                          tc.mutation_log_z_,
                          tc.mlog_enabled_flag_,
                          tc.debug_flag_},
              arena2_{DArena::map(ArenaConfig().with_name("arena2-ref")
                                  .with_size(tc.gc_size_ * tc.n_gen_)
                                  .with_store_header_flag(true))},
              gcos_{gcos_config_, &verify_stats_},
              mls_{mls_config_, &gcos_}
        {}
    }

    TEST_CASE("MutationLogStore-1", "[MutationLogStore]")
    {
        constexpr bool c_debug_flag = true;
        scope log0(XO_DEBUG(c_debug_flag), "MutationLogStore test");

        std::uint64_t seed = 7988747704879432247ul;
        //random_seed(&seed);
        log0 && log0(xtag("seed", seed));

        for (size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            auto rgen = xoshiro256ss(seed + i_tc);

            const Testcase & tc = s_testcase_v[i_tc];

            scope log1(XO_DEBUG(tc.debug_flag_),
                       "testcase loop",
                       xtag("i_tc", i_tc));

            INFO(tostr(xtag("i_tc", i_tc), xtag("n_tc", n_tc)));

            MlsFixture fixture(tc);

            // unlike GCObjectStore, separate init.
            //
            // TODO: adopt GCObjectStore pattern
            //
            fixture.mls_.init_mlogs(getpagesize());

            {
                MutationLog * to_0_mlog
                    = fixture.mls_.get_mlog(Role::to_space(),
                                            Generation::g0());
                MutationLog * from_0_mlog
                    = fixture.mls_.get_mlog(Role::from_space(),
                                            Generation::g0());
                MutationLog * triage_0_mlog
                    = fixture.mls_.triage_mlog(Generation::g0());

                REQUIRE(to_0_mlog);
                REQUIRE(from_0_mlog);
                REQUIRE(triage_0_mlog);
                REQUIRE(to_0_mlog != from_0_mlog);
                REQUIRE(to_0_mlog != triage_0_mlog);
                REQUIRE(from_0_mlog != triage_0_mlog);
            }

            {
                // updates counters in fixture.verify_stats_
                fixture.gcos_.verify_ok();
                fixture.mls_.verify_ok();

                INFO(tostr(xtag("n_gc_root", fixture.verify_stats_.n_gc_root_),
                           xtag("n_ext", fixture.verify_stats_.n_ext_),
                           xtag("n_from", fixture.verify_stats_.n_from_),
                           xtag("n_to", fixture.verify_stats_.n_to_)));
                INFO(tostr(xtag("n_fwd", fixture.verify_stats_.n_fwd_),
                           xtag("n_age_ok", fixture.verify_stats_.n_age_ok_),
                           xtag("n_age_bad", fixture.verify_stats_.n_age_bad_),
                           xtag("n_no_iface", fixture.verify_stats_.n_no_iface_)));

                REQUIRE(fixture.verify_stats_.is_ok());
            }

            GCObjectStore & gcos = fixture.gcos_;
            MutationLogStore & mls = fixture.mls_;

            {
                MutationLogEntry mentry;

                REQUIRE(mentry.parent() == nullptr);
                REQUIRE(mentry.p_data() == nullptr);
            }

            {
                // gcos setup.  parallels GCObjectStore.test.cpp
                {
                    REQUIRE(gcos.is_type_installed(typeseq::id<DList>()) == false);
                    REQUIRE(gcos.is_type_installed(typeseq::id<DBoolean>()) == false);
                    REQUIRE(gcos.is_type_installed(typeseq::id<DInteger>()) == false);

                    GcosTestutil::gcos_install_test_types(tc.do_type_registration_, &gcos);

                    if (tc.do_type_registration_) {
                        REQUIRE(gcos.is_type_installed(typeseq::id<DList>()) == true);
                        REQUIRE(gcos.is_type_installed(typeseq::id<DBoolean>()) == true);
                        REQUIRE(gcos.is_type_installed(typeseq::id<DInteger>()) == true);
                    }

                    GcosTestutil::gcos_verify_arena_partitioning(tc.n_gen_, tc.gc_size_, gcos);
                    GcosTestutil::gcos_verify_vacant(tc.n_gen_, tc.gc_size_, gcos);
                }
            }

            /** mutator/collector loop **/

            /** parallel {test,reference} object state.
             *
             **/
            std::vector<Recd> x1_v;
            std::vector<Recd> x2_v;

            for (uint32_t loop_index = 0; loop_index < tc.n_gc_loop_; ++loop_index) {
                scope log2(XO_DEBUG(tc.debug_flag_), "gc loop", xtag("loop_index", loop_index));

                INFO(xtag("loop_index", loop_index));

                GcosTestutil::gcos_construct_ab_object_graphs(tc.test_seq_,
                                                              tc.obj_graph_type_,
                                                              tc.n_i0_test_obj_,
                                                              tc.n_i0_test_assign_,
                                                              tc.n_i1_test_obj_,
                                                              tc.n_i1_test_assign_,
                                                              tc.debug_flag_,
                                                              &mls,
                                                              &gcos,
                                                              &fixture.arena2_,
                                                              loop_index,
                                                              &x1_v, &x2_v,
                                                              &rgen);

                Generation gk = Generation::g1();

                // no allocation errors
                REQUIRE(gcos.last_error().error_ == xo::mm::error::ok);

                GcosTestutil::gcos_verify_consistency(&gcos);

                // someday: print the graph. Need a cycle-detecting printer

                GcosTestutil::gcos_verify_ab_equivalence(x1_v, x2_v);
                GcosTestutil::gcos_verify_allocinfo(gcos, loop_index, x1_v);
                GcosTestutil::gcos_verify_gen0_only_allocated(tc.n_gen_, gcos, loop_index, x1_v);

                // swap roles for generations g < gk
                gcos.swap_roles(gk);
                mls.swap_roles(gk);

                GcosTestutil::gcos_verify_gen0_fromspace_only_allocated(tc.n_gen_, gcos, loop_index,
                                                                        gk, x1_v);

                // gc core: move stuff
                GcosTestutil::gcos_move_roots_and_verify(tc.do_type_registration_,
                                                         &gcos,
                                                         gk, x1_v, x2_v, tc.debug_flag_);

                DGCObjectStoreVisitor visitor(&gcos, gk);

                // after swapping roles only from-space mlog can be non-empty
                MlsTestutil::verify_fromspace_only_logged(mls, gk);

                // forward mutation log + mutation-rescued objects
                mls.forward_mutation_log(visitor.ref(), gk);

                // now only to-space mlog can be non-empty
                MlsTestutil::verify_tospace_only_logged(mls, gk);

                MlsTestutil::verify_mlog_load_bearing(mls, gk);

                // Might expect scanning generation g >= gk to confirm each object refs only to-space.
                //

                // reset (+ perhaps clean) from-space
                gcos.cleanup_phase(gk, tc.sanitize_flag());

                // scan {gcos, mls} to collect counters in *gcos.verify_stats()
                {
                    gcos.verify_stats()->clear();
                    gcos.verify_ok();
                    mls.verify_ok();

                    REQUIRE(gcos.verify_stats()->is_ok());
                }
            } /*one gc cycle per loop*/
        } /*testcase loop*/
    }
} /*namespace ut*/

/* end MutationLogStore.test.cpp */
