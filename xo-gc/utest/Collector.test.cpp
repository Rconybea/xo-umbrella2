/** @file Collector.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 *
 *  NOTE: properly unit testing gc behavior requires
 *        xo-object2 dependency;
 *        see xo-object2/utest
 **/

//#include <xo/alloc2/Allocator.hpp>
#include "random_allocs.hpp"
#include <xo/gc/X1Collector.hpp>
#include <xo/object2/Array.hpp>
#include <xo/object2/List.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/alloc2/CollectorTypeRegistry.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/randomgen/xoshiro256.hpp>
#include <xo/randomgen/random_seed.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <xo/indentlog/print/array.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::DList;
    using xo::scm::DArray;
    using xo::scm::DInteger;
    using xo::mm::CollectorTypeRegistry;
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::ICollector_Any;
    using xo::mm::AGCObject;
    using xo::mm::X1CollectorConfig;
    using xo::mm::DX1Collector;
    using xo::mm::Role;
    using xo::mm::ArenaConfig;
    using xo::mm::AllocHeaderConfig;
    using xo::mm::AllocHeader;
    using xo::mm::Generation;
    using xo::mm::c_max_generation;
    using xo::facet::DVariantPlaceholder;
    using xo::facet::with_facet;
    using xo::reflect::typeseq;
    using xo::scope;

    namespace ut {
        // checklist
        // - obj<ACollector> constructible              [ ]
        //   - obj<ACollector> truthy                   [ ]
        // - obj<ACollector,DX1Collector> constructible [ ]
        //
        // - obj<AAllocator,DX1Collector> constructible [ ]
        // - obj<AAllocator,DX1Collector> allocation

        TEST_CASE("collector-any-null", "[alloc2][gc][ACollector]")
        {
            /* empty variant collector */
            obj<ACollector> gc1;

            REQUIRE(!gc1._has_null_vptr());
            REQUIRE(!gc1);
            REQUIRE(gc1.iface() != nullptr);
            REQUIRE(gc1.data() == nullptr);
            REQUIRE(gc1._typeseq() == typeseq::id<DVariantPlaceholder>());
        }

        TEST_CASE("DX1Collector-1", "[alloc2][gc][DX1Collector]")
        {
            ArenaConfig arena_cfg = { .name_ = "_test_unused",
                                      .size_ = 4*1024*1024,
                                      .store_header_flag_ = true,
                                      .header_ = AllocHeaderConfig(0 /*guard_z*/,
                                                                   0xfd /*guard_byte*/,
                                                                   0 /*tseq_bits*/,
                                                                   0 /*age_bits*/,
                                                                   16 /*size_bits*/), };
            X1CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                      .n_generation_ = 2,
                                      .gc_trigger_v_ = {{64*1024, 1024*1024,
#ifdef NOPE
                                                         0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0
#endif
                                      }} };

            DX1Collector gc = DX1Collector{cfg};

            Generation g0 = Generation{0};
            REQUIRE(gc.to_space(g0));
            REQUIRE(gc.from_space(g0));
            REQUIRE(gc.to_space(g0)->is_mapped());
            REQUIRE(gc.from_space(g0)->is_mapped());

            Generation g1 = Generation{1};
            REQUIRE(gc.to_space(g1));
            REQUIRE(gc.from_space(g1));
            REQUIRE(gc.to_space(g1)->is_mapped());
            REQUIRE(gc.from_space(g1)->is_mapped());

            /* verify from/to x N/T are unique */
            REQUIRE(gc.to_space(g0) != gc.from_space(g0));
            REQUIRE(gc.to_space(g1) != gc.to_space(g0));
            REQUIRE(gc.from_space(g1) != gc.from_space(g0));
            REQUIRE(gc.to_space(g0) != gc.from_space(g1));
            REQUIRE(gc.to_space(g1) != gc.from_space(g1));

            for (Generation gi{0}; gi < 2; ++gi) {
                INFO(xtag("gi", gi));

                REQUIRE(gc.to_space(gi));
                REQUIRE(gc.from_space(gi));

                REQUIRE(gc.from_space(gi)->is_mapped());
                REQUIRE(gc.to_space(gi)->is_mapped());
            }

            for (Generation gi = Generation(2); gi < c_max_generation; ++gi) {
                INFO(xtag("gi", gi));

                REQUIRE(!gc.to_space(gi));
                REQUIRE(!gc.from_space(gi));
            }
        }

        TEST_CASE("collector-x1-obj", "[alloc2][gc]")
        {
            ArenaConfig arena_cfg = { .name_ = "_test_unused",
                                      .size_ = 4*1024*1024,
                                      .store_header_flag_ = true,
                                      .header_ = AllocHeaderConfig(0 /*guard_z*/,
                                                                   0xfd /*guard_byte*/,
                                                                   0 /*tseq_bits*/,
                                                                   0 /*age_bits*/,
                                                                   16 /*size_bits*/), };
            X1CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                      .n_generation_ = 2,
                                      .gc_trigger_v_ = {{64*1024, 1024*1024,
#ifdef NOPE
                                                         0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0
#endif
                                      }} };

            DX1Collector gc = DX1Collector{cfg};

            /* typed collector -- repr known at compile time */
            obj<ACollector, DX1Collector> x1(&gc);

            REQUIRE(!x1._has_null_vptr());
            REQUIRE(x1.iface());
            REQUIRE(x1.data());

            x1._drop();

            REQUIRE(x1.iface());
            REQUIRE(x1.data());
        }

        TEST_CASE("collector-x1-facet-mkobj", "[alloc2][gc]")
        {
            ArenaConfig arena_cfg = { .name_ = "_test_unused",
                                      .size_ = 4*1024*1024,
                                      .store_header_flag_ = true,
                                      .header_ = AllocHeaderConfig(0 /*guard_z*/,
                                                                   0xfd /*guard_byte*/,
                                                                   0 /*tseq-bits*/,
                                                                   0 /*age-bits*/,
                                                                   16 /*size-bits*/), };
            X1CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                      .n_generation_ = 2,
                                      .gc_trigger_v_ = {{64*1024, 1024*1024,
#ifdef NOPE
                                                         0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0
#endif
                                      }} };

            DX1Collector gc = DX1Collector{cfg};

            /* typed collector -- repr inferred at compile time */
            auto x1 = with_facet<ACollector>::mkobj(&gc);

            REQUIRE(x1.iface());
            REQUIRE(x1.data());

            x1._drop();

            REQUIRE(x1.iface());
            REQUIRE(x1.data());
        }

        TEST_CASE("collector-x1-alloc", "[alloc2][gc]")
        {
            scope log(XO_DEBUG(false), "DX1Collector alloc test");

            constexpr uint32_t c_n_alloc = 25;
            constexpr uint32_t c_reserved_z = 4*1024*1024;
            constexpr uint32_t c_max_alloc = c_reserved_z / c_n_alloc;
            // allowance for per-alloc overhead
            constexpr uint32_t c_max_alloc_payload = c_max_alloc - 32;

            static_assert(c_max_alloc > 0);
            static_assert(c_max_alloc_payload < c_max_alloc);

            ArenaConfig arena_cfg = { .name_ = "_test_unused",
                                      .size_ = 4*1024*1024,
                                      .store_header_flag_ = true,
                                      .header_ = AllocHeaderConfig(0 /*guard_z*/,
                                                                   0xfd /*guard-byte*/,
                                                                   0 /*tseq-bits*/,
                                                                   0 /*age-bits*/,
                                                                   16 /*size-bits*/), };

            /* collector with one generation collapses to a non-generational copying collector */
            X1CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                      .n_generation_ = 1,
                                      .gc_trigger_v_ = {{64*1024, 0,
#ifdef NOPE
                                                         0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0
#endif
                                      }} };

            DX1Collector x1state = DX1Collector{cfg};

            /* typed collector */
            auto x1gc = with_facet<ACollector>::mkobj(&x1state);
            auto x1alloc = with_facet<AAllocator>::mkobj(&x1state);

            REQUIRE(x1gc.iface());
            REQUIRE(x1gc.data());

            REQUIRE(x1alloc.iface());
            REQUIRE(x1alloc.data());

            rng::Seed<rng::xoshiro256ss> seed;
            log && log(xtag("seed", seed));

            auto rng = rng::xoshiro256ss(seed);

            bool catch_flag = false;
            REQUIRE(utest::AllocUtil::random_allocs(c_n_alloc, c_max_alloc_payload,
                                                    catch_flag, &rng, x1alloc));

            x1gc._drop();

            REQUIRE(x1gc.iface());
            REQUIRE(x1gc.data());
        }

        TEST_CASE("collector-x1-alloc2", "[alloc2][gc]")
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag),
                      "DX1Collector alloc test2");

            constexpr uint32_t c_n_alloc = 25;
            constexpr uint32_t c_reserved_z = 4*1024*1024;
            constexpr uint32_t c_max_alloc = c_reserved_z / c_n_alloc;
            // allowance for per-alloc overhead
            constexpr uint32_t c_max_alloc_payload = c_max_alloc - 32;

            ArenaConfig arena_cfg = {
                .name_ = "_test_unused",
                .size_ = c_reserved_z,
                .store_header_flag_ = true,
                .header_ = AllocHeaderConfig(8    /*guard_z*/,
                                             0xfd /*guard-byte*/,
                                             0    /*tseq-bits*/,
                                             0    /*age-bits*/,
                                             16   /*size-bits*/),
            };

            /* collector with one generation collapses to a non-generational copying collector */
            X1CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                      .n_generation_ = 1,
                                      .gc_trigger_v_ = {{64*1024, 0,
#ifdef NOPE
                                                         0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0,
                                                         0, 0, 0, 0
#endif
                                      }} };

            /* X1 allocator+collector */
            DX1Collector x1state = DX1Collector{cfg};

            /* typed collector i/face */
            auto x1gc = with_facet<ACollector>::mkobj(&x1state);
            /* typed allocator i/face */
            auto x1alloc = with_facet<AAllocator>::mkobj(&x1state);

            REQUIRE(x1gc.iface());
            REQUIRE(x1gc.data());

            REQUIRE(x1alloc.iface());
            REQUIRE(x1alloc.data());

            rng::Seed<rng::xoshiro256ss> seed;
            log && log("ratio: seed=", seed);

            auto rng = rng::xoshiro256ss(seed);

            // these are not gc-aware objects.
            // just testing ability to work as a low-level allocator
            REQUIRE(utest::AllocUtil::random_allocs(c_n_alloc, c_max_alloc_payload,
                                                    c_debug_flag, &rng, x1alloc));

            x1gc._drop();

            REQUIRE(x1gc.iface());
            REQUIRE(x1gc.data());
        }

        namespace {
            class Testcase {
            public:
                Testcase(uint32_t ng, uint32_t ns, size_t gcz, uint32_t otz, uint32_t xotz, bool dbg_flag)
                    : n_gen_{ng},
                      n_survive_{ns},
                      gc_halfspace_z_{gcz},
                      object_type_z_{otz},
                      expect_object_type_z_{xotz},
                      debug_flag_{dbg_flag}
                    {}

            public:
                /** number of generations in gco store **/
                uint32_t n_gen_ = 0;
                /** promote to next gen on surviving this number of gc cycles **/
                uint32_t n_survive_ = 0;
                /** size of each generations' half-space, in bytes **/
                size_t gc_halfspace_z_ = 0;
                /** storage for object-type array, in bytes
                 *  one 8-byte facet pointer per type
                 **/
                uint32_t object_type_z_;
#ifdef NOT_YET
                /** size for error output arena **/
                size_t error_size_ = 0;
#endif
                /** expected size of object-type array, in bytes, after orderly init **/
                uint32_t expect_object_type_z_ = 0;
                /** true to enable debug output for this test case **/
                bool debug_flag_ = false;
            };

            class X1Fixture {
            public:
                explicit X1Fixture(uint32_t i_tc, const Testcase & tc);

#ifdef NOT_IN_USE
                DArena error_arena_;
#endif
                DX1Collector gc_;
            };

            X1Fixture::X1Fixture(uint32_t i_tc, const Testcase & tc)
                : gc_(X1CollectorConfig()
                      .with_name("collector-x1-gc-" + std::to_string(i_tc))
                      .with_n_gen(tc.n_gen_)
                      .with_n_survive(tc.n_survive_)
                      .with_size(tc.gc_halfspace_z_)
                      .with_debug_flag(tc.debug_flag_))
            {
                auto gc = obj<ACollector,DX1Collector>(&gc_);

                // auto-install object types
                CollectorTypeRegistry::instance().install_types(gc);
            }

#          define nil nullptr
#          define T true
#          define F false

            static std::vector<Testcase> s_testcase_v = {
                /**
                 *                          debug_flag
                 *             expect_object_type_z  |
                 *               object_type_z    |  |
                 *        gc_halfspace_z     |    |  |
                 *  n_survive          |     |    |  |
                 *   n_gen  |          |     |    |  |
                 *       v  v          v     v    v  v
                 **/
                Testcase(1, 2, 16 * 1024,  128,  96, T),
            };

#          undef T
#          undef F
#          undef nil
        } /*namespace*/

        // full collector test.
        //
        // PLAN:
        // eventually: make generative
        //
        // Setup (
        // 1. gc_utest_main.cpp Subsystem::initialize_all()
        //    invokes per-module plugin init.  Gets types registered
        //    with FacetRegistry, CollectorTypeRegistry etc.
        // 2. per-utest collector setup (fixture)
        //    calls CollectorTypeRegistry::instance().install_types(gc)
        //    to establish the set of types that collector knows.
        //
        TEST_CASE("collector-x1-gc", "[alloc2][gc]")
        {
            const auto & testname = Catch::getResultCapture().getCurrentTestName();

            scope log(XO_DEBUG(true), xtag("test", testname));

            //std::uint64_t seed = 7988747704879432247ul;
            //random_seed(&seed);

            for (size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                // auto rgen = xoshiro256ss(seed + i_tc);

                const Testcase & tc = s_testcase_v[i_tc];

                scope log1(XO_DEBUG(tc.debug_flag_),
                           "testcase loop",
                           xtag("i_tc", i_tc));

                INFO(tostr(xtag("i_tc", i_tc), xtag("n_tc", n_tc)));

                X1Fixture fixture(i_tc, tc);

                auto & x1 = fixture.gc_;

                REQUIRE(x1.verify_ok());

                auto mm = x1.ref<AAllocator>();
                auto gc = mm.to_facet<ACollector>();

                REQUIRE(mm._typeseq() == typeseq::id<DX1Collector>());
                REQUIRE(gc._typeseq() == typeseq::id<DX1Collector>());

                Generation g0 = Generation::g0();

                // mm.allocated includes: { object-types, roots(=0), arenas(=0) }
                //
                REQUIRE(mm.allocated() == tc.expect_object_type_z_);
                REQUIRE(gc.allocated(g0, Role::to_space()) == 0);
                REQUIRE(gc.allocated(g0, Role::from_space()) == 0);

                Generation g1 = Generation::g1();
                {
                    auto roots = DArray::_empty(mm, 1)->ref<AGCObject>();
                    REQUIRE(mm->contains_allocated(Role::to_space(), roots.data()));

                    gc.add_gc_root(&roots);
                    {
                        auto x1 = DInteger::box(mm, 42);
                        auto x1_gco = obj<AGCObject>(x1);
                        auto l1 = DList::cons(mm, x1, DList::_nil());

                        REQUIRE(l1._typeseq() == typeseq::id<DList>());
                        REQUIRE(roots->push_back(mm, l1));
                        REQUIRE(mm->contains_allocated(Role::to_space(), x1.data()));
                        REQUIRE(mm->contains_allocated(Role::to_space(), l1.data()));

                        REQUIRE(roots->at(0) == l1);
                        REQUIRE(roots->at(0)._typeseq() == typeseq::id<DList>());

                        // z: total allocated so far
                        //   3x 8-byte header
                        //   sizeof(DInteger)
                        //   sizeof(DList)
                        //   sizeof(DArray(1))
                        //
                        auto z = (3 * sizeof(AllocHeader)
                                  + sizeof(DInteger)
                                  + sizeof(DList)
                                  + sizeof(DArray) + sizeof(obj<AGCObject>));
                        {
                            REQUIRE(z == 80);
                            // cf earlier assertion on mm.allocated();
                            // now adding cost of 3 specific objects
                            REQUIRE(mm.allocated() == tc.expect_object_type_z_ + z);
                            REQUIRE(gc.allocated(g0, Role::to_space()) == z);
                            REQUIRE(gc.allocated(g1, Role::to_space()) == 0);
                            REQUIRE(gc.allocated(g0, Role::from_space()) == 0);
                            REQUIRE(gc.allocated(g1, Role::from_space()) == 0);
                        }

                        gc->request_gc(g1);  // 1st GC

                        // x1 target got moved, og locn now relabeled from-space
                        REQUIRE(mm->contains(Role::from_space(), x1.data()));
                        REQUIRE(!mm->contains_allocated(Role::from_space(), x1.data()));
                        // l1 target got moved, og locn now relabeled from-space
                        REQUIRE(mm->contains(Role::from_space(), l1.data()));
                        REQUIRE(!mm->contains_allocated(Role::from_space(), l1.data()));

                        REQUIRE(mm.allocated() == tc.expect_object_type_z_ + z);
                        REQUIRE(gc.allocated(g0, Role::to_space()) == z);
                        REQUIRE(gc.allocated(g1, Role::to_space()) == 0);
                        REQUIRE(gc.allocated(g0, Role::from_space()) == 0);
                        REQUIRE(gc.allocated(g1, Role::from_space()) == 0);
                    }

                    // NOTE: if this fails:
                    //       look for preceding GCObjectStore::lookup_type out-of-bounds.
                    //       May need to add to CollectorTypeRegistry
                    //
                    REQUIRE(mm->contains_allocated(Role::to_space(), roots.data()));

                }
            }

#ifdef NOT_YET
            /* typed collector i/face */
            auto x1gc = with_facet<ACollector>::mkobj(&x1state);
            /* typed allocator i/face */
            auto x1alloc = with_facet<AAllocator>::mkobj(&x1state);

            REQUIRE(x1gc.iface());
            REQUIRE(x1gc.data());

            REQUIRE(x1alloc.iface());
            REQUIRE(x1alloc.data());

            rng::Seed<rng::xoshiro256ss> seed;
            log && log("ratio: seed=", seed);

            auto rng = rng::xoshiro256ss(seed);

            // these are not gc-aware objects.
            // just testing ability to work as a low-level allocator
            REQUIRE(utest::AllocUtil::random_allocs(25, false, &rng, x1alloc));
#endif
        }
    }
}

/* end Collector.test.cpp */
