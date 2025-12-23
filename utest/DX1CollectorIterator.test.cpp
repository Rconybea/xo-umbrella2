/** @file DX1CollectorIterator.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include <xo/alloc2/Allocator.hpp>
#include "AllocIterator.hpp"
#include "DX1CollectorIterator.hpp"
#include "detail/IAllocator_DX1Collector.hpp"
#include "detail/IAllocIterator_DX1CollectorIterator.hpp"
#include "arena/ArenaConfig.hpp"
#include "padding.hpp"
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::AAllocator;
    using xo::mm::AAllocIterator;
//    using xo::mm::IAllocIterator_Any;
    using xo::mm::IAllocIterator_Xfer;
    using xo::mm::IAllocIterator_DX1CollectorIterator;
    using xo::mm::DX1Collector;
    using xo::mm::DX1CollectorIterator;
    using xo::mm::DArena;
    using xo::mm::DArenaIterator;
    using xo::mm::CollectorConfig;
    using xo::mm::ArenaConfig;
    using xo::mm::AllocHeaderConfig;
    using xo::mm::cmpresult;
    using xo::mm::padding;
    using xo::facet::with_facet;
    using std::byte;

    namespace ut {
        TEST_CASE("IAllocIterator_Xfer_DX1CollectorIterator", "[alloc2]")
        {
            /* verify IAllocIterator_Xfer is constructible + satisfies concept checks */
            IAllocIterator_Xfer<DX1CollectorIterator, IAllocIterator_DX1CollectorIterator> xfer;
            REQUIRE(IAllocIterator_Xfer<DX1CollectorIterator, IAllocIterator_DX1CollectorIterator>::_valid);
        }

        TEST_CASE("DX1CollectorIterator-1", "[alloc2][gc][DX1Collector]")
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

            /* verify obj 'fat pointer' packaging */
            obj<AAllocIterator,DX1CollectorIterator> ix_vt{&ix};
            obj<AAllocIterator,DX1CollectorIterator> end_ix_vt{&end_ix};

            REQUIRE(ix_vt.iface());
            REQUIRE(ix_vt.data());
            REQUIRE(end_ix_vt.iface());
            REQUIRE(end_ix_vt.data());

            cmpresult cmp = ix_vt.compare(end_ix_vt);

            REQUIRE(cmp.is_equal());
            REQUIRE(ix_vt == end_ix_vt);
        }

        TEST_CASE("DX1CollectorIterator-2", "[alloc2][gc][DX1Collector]")
        {
            scope log(XO_DEBUG(false));

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
            obj<AAllocator, DX1Collector> a1o{&gc};

            REQUIRE(a1o.reserved() >= arena_cfg.size_);
            REQUIRE(a1o.committed() == 0);
            REQUIRE(a1o.available() == 0);
            REQUIRE(a1o.allocated() == 0);

            size_t req_z = 13;
            byte * mem = gc.alloc(req_z);

            REQUIRE(mem != nullptr);

            log && log("should have iterators separated by one alloc");

            {
                auto     ix = gc.begin();
                auto end_ix = gc.end();

                REQUIRE(ix.is_valid());
                REQUIRE(end_ix.is_valid());
                REQUIRE(ix != end_ix);

                /* verify obj 'fat pointer' packaging */
                auto     ix_vt = with_facet<AAllocIterator>::mkobj(&ix);
                auto end_ix_vt = with_facet<AAllocIterator>::mkobj(&end_ix);

                REQUIRE(ix_vt.iface());
                REQUIRE(ix_vt.data());
                REQUIRE(end_ix_vt.iface());
                REQUIRE(end_ix_vt.data());

                cmpresult cmp = ix_vt.compare(end_ix_vt);

                REQUIRE(cmp.is_lesser());
                REQUIRE(ix_vt != end_ix_vt);

                /* we only did one alloc, should be able
                 * to visit it
                 */
                auto info = ix_vt.deref();

                REQUIRE(info.is_valid());
                REQUIRE(info.payload().first == mem);
                REQUIRE(info.size() == padding::with_padding(req_z));

                ix_vt.next();

                log && log(xtag("ix.gen", ix.gen_ix()),
                           xtag("ix.arena_ix.arena", ix.arena_ix().arena_));
                log && log(xtag("end_ix.gen", end_ix.gen_ix()),
                           xtag("end_ix.arena_ix.arena", end_ix.arena_ix().arena_));

                REQUIRE(ix_vt == end_ix_vt);
            }

            {
                //auto range = gc.alloc_range

                DArena scratch_mm
                    = DArena::map(
                        ArenaConfig{
                            .size_ = 4*1024,
                            .hugepage_z_ = 4*1024});

                auto range = a1o.alloc_range(scratch_mm);

                obj<AAllocIterator> ix = range.begin();
                obj<AAllocIterator> end_ix = range.end();

                REQUIRE(ix.iface());
                REQUIRE(ix.data());
                REQUIRE(end_ix.iface());
                REQUIRE(end_ix.data());

                REQUIRE(scratch_mm.allocated() >= 2*sizeof(DArenaIterator));
                REQUIRE(scratch_mm.available() > 0);

                REQUIRE(ix.compare(ix).is_equal());

                REQUIRE(ix != end_ix);

                {
                    REQUIRE(ix.deref().is_valid());
                    REQUIRE(ix.deref().size() == padding::with_padding(req_z));

                    auto [payload_lo, payload_hi] = ix.deref().payload();

                    REQUIRE(payload_lo == mem);
                    REQUIRE(payload_hi == mem + ix.deref().size());
                }

                {
                    ++ix;

                    REQUIRE(ix == end_ix);
                }
            }

            // repeat, this time using range iteration
            {
                DArena scratch_mm
                    = DArena::map(
                        ArenaConfig{
                            .size_ = 4*1024,
                            .hugepage_z_ = 4*1024});

                for (const auto & info : a1o.alloc_range(scratch_mm)) {
                    REQUIRE(info.is_valid());
                    REQUIRE(info.size() == padding::with_padding(req_z));
                    REQUIRE(info.payload().first == mem);
                    REQUIRE(info.payload().second == mem + info.size());
                }
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end DX1CollectorIterator.test.cpp */
