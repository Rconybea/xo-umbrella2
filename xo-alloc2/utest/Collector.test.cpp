/** @file Collector.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 *
 *  NOTE: properly unit testing gc behavior requires
 *        xo-object2 dependency;
 *        see xo-object2/utest
 **/

#include "Collector.hpp"
#include "gc/ICollector_DX1Collector.hpp"
//#include "gc/DX1Collector.hpp"
#include <xo/indentlog/print/tag.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::ACollector;
    using xo::mm::CollectorConfig;
    using xo::mm::DX1Collector;
    using xo::mm::ArenaConfig;
    using xo::mm::generation;
    using xo::mm::c_max_generation;

    namespace ut {
        // checklist
        // - obj<ACollector> constructible [ ]
        //   - obj<ACollector> truthy      [ ]

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
                                      .header_size_mask_ = 0x0000ffff, };
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
                                      .header_size_mask_ = 0x0000ffff, };
            CollectorConfig cfg = { .arena_config_ = arena_cfg,
                                    .n_generation_ = 2,
                                    .gc_trigger_v_ = {{64*1024, 1024*1024, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0,
                                                       0, 0, 0, 0}} };

            DX1Collector gc = DX1Collector{cfg};

            /* typed collector */
            obj<ACollector, DX1Collector> x1(&gc);

        }
    }
}

/* end Collector.test.cpp */
