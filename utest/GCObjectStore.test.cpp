/** @file GCObjectStore.test.cpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#include "GcosTestutil.hpp"
#include <xo/gc/GCObjectStore.hpp>
#include <xo/gc/GCObjectStoreVisitor.hpp>
#include <xo/gc/X1VerifyStats.hpp>
#include <xo/object2/ListOps.hpp>
#include <xo/object2/List.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/Boolean.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/facet/TypeRegistry.hpp>
#include <xo/arena/print.hpp>
#include <xo/arena/backtrace.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <xo/randomgen/xoshiro256.hpp>
#include <xo/randomgen/random_seed.hpp>
#include <catch2/catch.hpp>

namespace ut {
    using xo::scm::ListOps;
    using xo::scm::DList;
    using xo::scm::DInteger;
    using xo::scm::DBoolean;
    using xo::mm::GCObjectStoreConfig;
    using xo::mm::GCObjectStore;
    using xo::mm::X1VerifyStats;
    using xo::mm::AGCObject;
    using xo::mm::AGCObjectVisitor;
    using xo::mm::AGCObjectVisitor;
    using xo::mm::DGCObjectStoreVisitor;
    using xo::mm::Generation;
    using xo::mm::Role;
    using xo::mm::object_age;
    using xo::mm::ArenaConfig;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::AllocInfo;
    using xo::mm::c_max_generation;
    using xo::print_backtrace_dwarf;
    using xo::facet::obj;
    using xo::facet::TypeRegistry;
    using xo::facet::typeseq;
    using xo::facet::impl_for;
    using xo::rng::xoshiro256ss;
    using xo::rng::random_seed;
    using xo::scope;
    using xo::xtag;
    using xo::tostr;
    using std::size_t;
    using std::uint32_t;

    namespace {
        struct Testcase {
            explicit Testcase(uint32_t n_gen, uint32_t n_survive,
                              size_t gc_z, uint32_t type_z,
                              bool do_type_registration,
                              size_t report_z,
                              size_t error_z,
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
                  report_size_{report_z},
                  error_size_{error_z},
                  obj_graph_type_{obj_graph_type},
                  n_gc_loop_{n_gc_loop},
                  n_i0_test_obj_{n_i0_test_obj},
                  n_i0_test_assign_{n_i0_test_assign},
                  n_i1_test_obj_{n_i1_test_obj},
                  n_i1_test_assign_{n_i1_test_assign},
                  debug_flag_{debug_flag}
                {}

            /** number of generations in gco store **/
            uint32_t n_gen_ = 0;
            /** object promotes on surviving this many gc cycles **/
            uint32_t n_survive_ = 0;
            /** size of each generation's half-space, in bytes **/
            size_t gc_size_ = 0;
            /** Storage for object type array, in bytes.
             *  (need to allow 1 pointer per type)
             **/
            uint32_t object_type_z_ = 0;

            /** if true, register types for
             *  gc-aware types used in unit test
             *  (i.e. DBoolean)
             **/
            bool do_type_registration_ = false;
            /** size for report-output arena **/
            size_t report_size_ = 0;
            /** size for error-output arena **/
            size_t error_size_ = 0;
            /** object graph type **/
            TestGraphType obj_graph_type_ = TestGraphType::random;
            /** #of gc-like "move all the roots" phases to perform **/
            uint32_t n_gc_loop_ = 0;
            /** first loop: #of cells in random object graph **/
            uint32_t n_i0_test_obj_ = 0;
            /** first loop: #of random assignments to attempt (these may create cycles, for example) **/
            uint32_t n_i0_test_assign_ = 0;
            /** 2nd+later loop: #of cells in random object graph **/
            uint32_t n_i1_test_obj_ = 0;
            /** 2nd+later loop: #of random assignments to attempt **/
            uint32_t n_i1_test_assign_ = 0;

            /** true to enable debug when attempting this test case **/
            bool debug_flag_ = false;
        };

        constexpr TestGraphType c_selfcycle = TestGraphType::selfcycle;
        constexpr TestGraphType c_random = TestGraphType::random;
        /** arena size for object age/type reports **/
        constexpr uint32_t c_report_z1 = 64 * 1024;
        constexpr uint32_t c_error_z1 = 16 * 1024;

#      define T true
#      define F false

        static std::vector<Testcase> s_testcase_v = {
            // note: report_z: 64k not sufficient for report_object_ages()

            /** n_gen, n_survive, gc_size, object_type_z, do_type_registration,
             *  report_z, error_z,
             *  n_gc_loop,
             *  n_i0_obj, n_i0_test_assign,
             *  n_i1_obj, n_i1_test_assign,
             *  debug_flag
             *                                                                               n_i1_obj
             *                                                                   n_i0_test_assign   |
             *                                                                       n_i0_obj   |   |
             *                                                                  n_gc_loop   |   |   |
             *                                                                          v   v   v   v
             **/
            Testcase(2, 4, 16 * 1024, 8 * 128, F, c_report_z1, c_error_z1, c_random,    1,  0,  0,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_selfcycle, 1,  1,  0,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_selfcycle, 3,  1,  0,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_selfcycle, 4,  1,  0,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_random,    1,  1,  0,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_random,    2,  1,  0,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_random,    4,  1,  0,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_random,    8,  1,  0,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_random,    1,  2, 13,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_random,    1,  2, 25,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_random,    1,  5,  0,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_random,    1,  4,  2,  0, 0, F),
            Testcase(2, 4, 16 * 1024, 8 * 128, T, c_report_z1, c_error_z1, c_random,    1, 50, 25,  0, 0, F),
        };

#      undef T
#      undef F

    } /*namespace*/

    namespace {
        // aux functions specific to GCObjectStore-1 unit test below

        /** Fixture for GCObjectStore-1 test.
         *  Compare similar but not identical fixture in MutationLogStore.test.cpp
         **/
        class GcosFixture {
        public:
            explicit GcosFixture(const Testcase & tc);

            auto report_mm() { return obj<AAllocator,DArena>(&report_arena_); }
            auto error_mm() { return obj<AAllocator,DArena>(&error_arena_); }

            /** configuration for @ref gcos_ **/
            GCObjectStoreConfig gcos_config_;

            /** Parallel arena for reference
             *
             *  We will allocate parallel object model in this arena
             *  for reference; then compare with GCObjectStore behavior.
             *
             *  1. arena2 doesn't have any generation layer cake stuff
             *  2. arena2 doesn't have concept of installed types.
             *     It doesn't have or require any builtin ability to traverse an object model
             **/
            DArena arena2_;

            /** Arena for holding report output:
             *  See GCObjectStore methods .report_object_types(), .report_object_ages()
             **/
            DArena report_arena_;
            /** Arena for holding error messages **/
            DArena error_arena_;

            /** statistics collected by GCObjectStore.verify_ok() **/
            X1VerifyStats verify_stats_;

            /** the thing we're exercising using this fixture **/
            GCObjectStore gcos_;
        };

        GcosFixture::GcosFixture(const Testcase & tc)
        : gcos_config_{ArenaConfig()
                       .with_name("gcos-fixture-arena-name-notused")
                       .with_size(tc.gc_size_)
                       .with_store_header_flag(true),
                       tc.n_gen_,
                       tc.n_survive_,
                       tc.object_type_z_,
                       tc.debug_flag_},
          arena2_{DArena::map(ArenaConfig().with_name("arena2-ref")
                              .with_size(tc.gc_size_ * tc.n_gen_)
                              .with_store_header_flag(true))},
          report_arena_{DArena::map(ArenaConfig().with_name("report-arena")
                                    .with_size(tc.report_size_)
                                    .with_store_header_flag(true))},
          error_arena_{DArena::map(ArenaConfig().with_name("error-arena")
                                   .with_size(tc.error_size_)
                                   .with_store_header_flag(true))},
          gcos_{gcos_config_, &verify_stats_}
        {}

    }

    TEST_CASE("GCObjectStore-1", "[GCObjectStore]")
    {
        constexpr bool c_debug_flag = false;
        scope log0(XO_DEBUG(c_debug_flag), "GCObjectStore test");

        std::uint64_t seed = 12168164826603821466ul;
        //random_seed(&seed);
        log0 && log0(xtag("seed", seed));

        for (size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
            // Loop iterations here are independent.
            // Could execute test cases in any order

            // deterministic seed choice for each testcase
            // -> individual cases preserve rng behavior
            // regardless of testcase order and/or subsetting

            auto rgen = xoshiro256ss(seed + i_tc);

            const Testcase & tc = s_testcase_v[i_tc];

            scope log1(XO_DEBUG(tc.debug_flag_), "testcase loop", xtag("i_tc", i_tc));

            INFO(tostr(xtag("i_tc", i_tc), xtag("n_tc", n_tc)));

            GcosFixture fixture(tc);

            GCObjectStore & gcos = fixture.gcos_;

            REQUIRE(gcos.is_type_installed(typeseq::id<DList>()) == false);
            REQUIRE(gcos.is_type_installed(typeseq::id<DBoolean>()) == false);

            GcosTestutil::gcos_install_test_types(tc.do_type_registration_, &gcos);
            GcosTestutil::gcos_verify_arena_partitioning(tc.n_gen_, tc.gc_size_, gcos);
            GcosTestutil::gcos_verify_vacant(tc.n_gen_, tc.gc_size_, gcos);

            // create object(s).
            // details depend on test case

            std::vector<Recd> x1_v;
            std::vector<Recd> x2_v;

            for(uint32_t loop_index = 0; loop_index < tc.n_gc_loop_; ++loop_index) {
                scope log2(XO_DEBUG(tc.debug_flag_), "gc loop", xtag("loop_index", loop_index));

                // construct, extend, and/or modify object graphs in {x1_v, x2_v}

                GcosTestutil::gcos_construct_ab_object_graphs(TestSequence{} /*test_seq*/,
                                                              tc.obj_graph_type_,
                                                              tc.n_i0_test_obj_,
                                                              tc.n_i0_test_assign_,
                                                              tc.n_i1_test_obj_,
                                                              tc.n_i1_test_assign_,
                                                              tc.debug_flag_,
                                                              nullptr /*p_mls*/,
                                                              &gcos,
                                                              &fixture.arena2_,
                                                              loop_index,
                                                              &x1_v, &x2_v,
                                                              &rgen);

                // no allocation errors
                REQUIRE(gcos.last_error().error_ == xo::mm::error::ok);

                log1 && log1("verify before any gcos side effects");

                GcosTestutil::gcos_verify_consistency(&gcos);

                // someday: print the graph. Need a cycle-detecting printer

                GcosTestutil::gcos_verify_ab_equivalence(x1_v, x2_v);
                GcosTestutil::gcos_verify_allocinfo(gcos, loop_index, x1_v);
                GcosTestutil::gcos_verify_gen0_only_allocated(tc.n_gen_, gcos, loop_index, x1_v);

                // swap_roles [but only for generation < g1, i.e. g0
                gcos.swap_roles(Generation::g1());

                GcosTestutil::gcos_verify_gen0_fromspace_only_allocated(tc.n_gen_, gcos, loop_index,
                                                                        Generation::g1(), x1_v);

                GcosTestutil::gcos_move_roots_and_verify(tc.do_type_registration_,
                                                         &gcos,
                                                         Generation::g1(), x1_v, x2_v, tc.debug_flag_);

                // Things to test:
                // - deep_move_interior()   // used from MutationLogStore
                // - forward_inplace_aux()  // used from DX1Collector.visit_child

                {
                    bool sanitize_flag = true;

                    // swaps to- and from- spaces again
                    // Now from-space will be empty, all live objects in to-space

                    gcos.cleanup_phase(Generation::g1(), sanitize_flag);
                }

                {
                    fixture.verify_stats_.clear();

                    // traverses stored objects, updates counters
                    // in verify_stats (= gco.p_verify_stats_, via ctor)
                    //
                    gcos.verify_ok();

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

                // report stats by type
                {
                    obj<AGCObject> report_gco;
                    bool ok = gcos.report_object_types(fixture.report_mm(), fixture.error_mm(), &report_gco);

                    REQUIRE(ok);
                    REQUIRE(report_gco);

                    // TODO: print report_gco, verify output

                    // discard report

                    report_gco.reset();
                    fixture.report_mm()->clear();
                }

                // report stats by age
                {
                    obj<AGCObject> report_gco;
                    bool ok = gcos.report_object_ages(fixture.report_mm(), fixture.error_mm(), &report_gco);

                    if (!ok) {
                        log1.retroactively_enable();
                        log1 && log1(xtag("error", fixture.report_mm().last_error()));
                    }

                    REQUIRE(ok);
                    REQUIRE(report_gco);

                    // TODO: print report_gco, verify output

                    // discard report

                    report_gco.reset();
                    fixture.report_mm()->clear();
                }

                // operate visitor (loose ends revealed by coverage).
                // mostly tested by moving objects
                {
                    DGCObjectStoreVisitor visitor(&gcos, Generation::g0());
                    auto visitor_fop = obj<AGCObjectVisitor,DGCObjectStoreVisitor>(&visitor);

                    REQUIRE(!visitor_fop.iface()->_has_null_vptr());
                    REQUIRE(visitor_fop._typeseq() == typeseq::id<DGCObjectStoreVisitor>());

                    visitor_fop._drop();
                }
            }
        } /* loop over test cases */
    } /* TEST_CASE(GCObjectStore-1) */

} /*namespace ut*/

/* end GCObjectStore.test.cpp */
