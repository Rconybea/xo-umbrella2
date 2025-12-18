/** @file DX1CollectorIterator.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "Allocator.hpp"
#include "AllocIterator.hpp"
#include "gc/DX1CollectorIterator.hpp"
#include "gc/IAllocator_DX1Collector.hpp"
#include "gc/IAllocIterator_DX1CollectorIterator.hpp"
#include "arena/ArenaConfig.hpp"
#include "padding.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::AAllocIterator;
    using xo::mm::IAllocIterator_Any;
    using xo::mm::IAllocIterator_Xfer;
    using xo::mm::IAllocIterator_DX1CollectorIterator;
    using xo::mm::DX1Collector;
    using xo::mm::DX1CollectorIterator;
    using xo::mm::CollectorConfig;
    using xo::mm::ArenaConfig;
    using xo::mm::AllocHeaderConfig;

    namespace ut {
        TEST_CASE("IAllocIterator_Xfer_DX1CollectorIterator", "[alloc2]")
        {
            /* verify IAllocIterator_Xfer is constructible + satisfies concept checks */
            IAllocIterator_Xfer<DX1CollectorIterator, IAllocIterator_DX1CollectorIterator> xfer;
            REQUIRE(IAllocIterator_Xfer<DX1CollectorIterator, IAllocIterator_DX1CollectorIterator>::_valid);
        }

        TEST_CASE("DX1CollectorIterator", "[alloc2][gc][DX1Collector]")
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

            auto ix = gc.begin();
            auto end_ix = gc.end();

            REQUIRE(ix.is_valid());
            REQUIRE(end_ix.is_valid());

            REQUIRE(ix == end_ix);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end DX1CollectorIterator.test.cpp */
