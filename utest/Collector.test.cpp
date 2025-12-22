/** @file Collector.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 *
 *  NOTE: properly unit testing gc behavior requires
 *        xo-object2 dependency;
 *        see xo-object2/utest
 **/

#include "Allocator.hpp"
#include "Collector.hpp"
#include "random_allocs.hpp"
#include "gc/ICollector_DX1Collector.hpp"
#include "gc/IAllocator_DX1Collector.hpp"
//#include "gc/DX1Collector.hpp"
#include <xo/randomgen/xoshiro256.hpp>
#include <xo/randomgen/random_seed.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <xo/indentlog/print/array.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::CollectorConfig;
    using xo::mm::DX1Collector;
    using xo::mm::ArenaConfig;
    using xo::mm::AllocHeaderConfig;
    using xo::mm::generation;
    using xo::mm::c_max_generation;
    using xo::facet::with_facet;
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

            REQUIRE(!gc1);
            REQUIRE(gc1.iface() != nullptr);
            REQUIRE(gc1.data() == nullptr);
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
            CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                    .n_generation_ = 2,
                                    .gc_trigger_v_ = {{64*1024, 1024*1024, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0}} };

            DX1Collector gc = DX1Collector{cfg};

            generation g0 = generation{0};
            REQUIRE(gc.to_space(g0));
            REQUIRE(gc.from_space(g0));
            REQUIRE(gc.to_space(g0)->is_mapped());
            REQUIRE(gc.from_space(g0)->is_mapped());

            generation g1 = generation{1};
            REQUIRE(gc.to_space(g1));
            REQUIRE(gc.from_space(g1));
            REQUIRE(gc.to_space(g1)->is_mapped());
            REQUIRE(gc.from_space(g1)->is_mapped());

            /* verify from/to x N/T are unique */
            REQUIRE(gc.to_space(g0) != gc.from_space(g0));
            REQUIRE(gc.to_space(g1) != gc.to_space(g0));
            REQUIRE(gc.from_space(g1) != gc.from_space(g0));
            REQUIRE(gc.to_space(g0) != gc.from_space(g1));

            for (generation gi = generation(2); gi < c_max_generation; ++gi) {
                INFO(xtag("gi", gi));

                REQUIRE(!gc.to_space(gi));
                REQUIRE(!gc.from_space(gi));

                REQUIRE(!gc.space_storage_[0][gi].is_mapped());
                REQUIRE(!gc.space_storage_[1][gi].is_mapped());
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
            CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                    .n_generation_ = 2,
                                    .gc_trigger_v_ = {{64*1024, 1024*1024, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0}} };

            DX1Collector gc = DX1Collector{cfg};

            /* typed collector -- repr known at compile time */
            obj<ACollector, DX1Collector> x1(&gc);

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
            CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                    .n_generation_ = 2,
                                    .gc_trigger_v_ = {{64*1024, 1024*1024, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0}} };

            DX1Collector gc = DX1Collector{cfg};

            /* typed collector -- repr inferred at compile time */
            auto x1 = with_facet<ACollector>::mkobj(&gc);

            REQUIRE(x1.iface());
            REQUIRE(x1.data());
        }

        TEST_CASE("collector-x1-alloc", "[alloc2][gc]")
        {
            scope log(XO_DEBUG(false), "DX1Collector alloc test");

            ArenaConfig arena_cfg = { .name_ = "_test_unused",
                                      .size_ = 4*1024*1024,
                                      .store_header_flag_ = true,
                                      .header_ = AllocHeaderConfig(0 /*guard_z*/,
                                                                   0xfd /*guard-byte*/,
                                                                   0 /*tseq-bits*/,
                                                                   0 /*age-bits*/,
                                                                   16 /*size-bits*/), };

            /* collector with one generation collapses to a non-generational copying collector */
            CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                    .n_generation_ = 1,
                                    .gc_trigger_v_ = {{64*1024, 0, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0}} };

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
            REQUIRE(utest::AllocUtil::random_allocs(25, catch_flag, &rng, x1alloc));
        }

        TEST_CASE("collector-x1-alloc2", "[alloc2][gc]")
        {
            scope log(XO_DEBUG(false),
                      "DX1Collector alloc test2");

            ArenaConfig arena_cfg = { .name_ = "_test_unused",
                                      .size_ = 4*1024*1024,
                                      .store_header_flag_ = true,
                                      .header_ = AllocHeaderConfig(8    /*guard_z*/,
                                                                   0xfd /*guard-byte*/,
                                                                   0    /*tseq-bits*/,
                                                                   0    /*age-bits*/,
                                                                   16   /*size-bits*/),
            };

            /* collector with one generation collapses to a non-generational copying collector */
            CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                    .n_generation_ = 1,
                                    .gc_trigger_v_ = {{64*1024, 0, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0}} };

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

            REQUIRE(utest::AllocUtil::random_allocs(25, false, &rng, x1alloc));
        }
    }
}

/* end Collector.test.cpp */
