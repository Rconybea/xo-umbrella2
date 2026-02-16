/** @file AllocIterator.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "Allocator.hpp"
#include "AllocIterator.hpp"
#include "arena/IAllocator_DArena.hpp"
#include "arena/IAllocIterator_DArenaIterator.hpp"
#include "padding.hpp"
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::AAllocator;

    using xo::mm::AllocInfo;

    using xo::mm::AAllocIterator;
    using xo::mm::IAllocIterator_Any;
    using xo::mm::IAllocIterator_Xfer;
    using xo::mm::IAllocIterator_DArenaIterator;
    using xo::mm::DArenaIterator;

    using xo::mm::ArenaConfig;
    using xo::mm::DArena;

    using xo::mm::padding;
    using xo::mm::error;

    using xo::facet::DVariantPlaceholder;
    using xo::facet::obj;
    using xo::facet::typeseq;

    using std::byte;

    namespace ut {
        TEST_CASE("IAllocIterator_Xfer_DArenaIterator", "[alloc2]")
        {
            /* verify IAllocIterator_Xfer is constructible + satisfies concept checks */
            IAllocIterator_Xfer<DArenaIterator, IAllocIterator_DArenaIterator> xfer;
            REQUIRE(IAllocIterator_Xfer<DArenaIterator, IAllocIterator_DArenaIterator>::_valid);
        }

        TEST_CASE("IAllocIterator_Any", "[alloc2]")
        {
            /* verify IAllocIterator_Any is constructible + satisfies concept checks */
            IAllocIterator_Any any;
            REQUIRE(IAllocIterator_Any::_valid);
        }

        TEST_CASE("obj_IAllocIterator", "[alloc2]")
        {
            /* verify variant obj constructible */
            obj<AAllocIterator> obj_any;
            REQUIRE(obj_any.iface());
            REQUIRE(obj_any.data() == nullptr);
        }

        TEST_CASE("IAllocIterator-disabled-1", "[alloc2]")
        {
            /* verify iteration over empty arena */
            /* typed allocator a1o */
            ArenaConfig cfg { .name_       = "testarena",
                              .size_       = 64*1024,
                              .debug_flag_ = false };
            DArena arena = DArena::map(cfg);
            obj<AAllocator, DArena> a1o{&arena};

            REQUIRE(a1o.reserved() >= cfg.size_);
            REQUIRE(a1o.committed() == 0);
            REQUIRE(a1o.available() == 0);
            REQUIRE(a1o.allocated() == 0);

            DArenaIterator ix = arena.begin();
            /* iteration not supported since we did not set
             * ArenaConfig.store_header_flag_
             */
            REQUIRE(ix.is_invalid());
            REQUIRE(ix != ix);
            REQUIRE(arena.error_count_ == 1);
            REQUIRE(arena.last_error_.error_seq_ == 1);
            REQUIRE(arena.last_error_.error_ == error::alloc_iterator_not_supported);

            DArenaIterator end_ix = arena.end();
            /* iteration not supported since we did not set
             * ArenaConfig.store_header_flag_
             */
            REQUIRE(end_ix.is_invalid());
            REQUIRE(end_ix != end_ix);
            REQUIRE(arena.error_count_ == 2);
            REQUIRE(arena.last_error_.error_seq_ == 2);
            REQUIRE(arena.last_error_.error_ == error::alloc_iterator_not_supported);
        }

        TEST_CASE("IAllocIterator-emptyarena", "[alloc2]")
        {
            /* verify iteration over empty arena */
            /* typed allocator a1o */
            ArenaConfig cfg { .name_              = "testarena",
                              .size_              = 64*1024,
                              .store_header_flag_ = true,
                              .debug_flag_        = false };
            DArena arena = DArena::map(cfg);
            obj<AAllocator, DArena> a1o{&arena};

            REQUIRE(a1o.reserved() >= cfg.size_);
            REQUIRE(a1o.committed() == 0);
            REQUIRE(a1o.available() == 0);
            REQUIRE(a1o.allocated() == 0);

            DArenaIterator     ix = arena.begin();
            DArenaIterator end_ix = arena.end();

            REQUIRE(ix.is_valid());
            REQUIRE(end_ix.is_valid());

            /* verify obj 'fat pointer' packaging */
            obj<AAllocIterator,DArenaIterator> ix_vt{&ix};
            obj<AAllocIterator,DArenaIterator> end_ix_vt{&end_ix};

            REQUIRE(ix_vt.iface());
            REQUIRE(ix_vt.data());
            REQUIRE(end_ix_vt.iface());
            REQUIRE(end_ix_vt.data());

            /* arena is empty, so begin==end */
            REQUIRE(ix == end_ix);

            REQUIRE(arena.error_count_ == 0);

            /* empty iterator cannot be dereferenced */
            {
                AllocInfo bad_info = *ix;
                REQUIRE(!bad_info.is_valid());

                REQUIRE(arena.error_count_ == 1);
                REQUIRE(arena.last_error_.error_seq_ == 1);
                REQUIRE(arena.last_error_.error_ == error::alloc_iterator_deref);
            }

            /* empty iterator cannot be advanced */
            {
                ix.next();

                REQUIRE(arena.error_count_ == 2);
                REQUIRE(arena.last_error_.error_seq_ == 2);
                REQUIRE(arena.last_error_.error_ == error::alloc_iterator_next);
            }
        }

        TEST_CASE("IAllocIterator-singlearena", "[alloc2]")
        {
            scope log(XO_DEBUG(false));

            ArenaConfig cfg { .name_              = "testarena",
                              .size_              = 64*1024,
                              .store_header_flag_ = true,
                              .debug_flag_        = false };
            DArena arena = DArena::map(cfg);
            obj<AAllocator, DArena> a1o{&arena};

            REQUIRE(arena.error_count_ == 0);
            REQUIRE(a1o.reserved() >= cfg.size_);
            REQUIRE(a1o.committed() == 0);
            REQUIRE(a1o.available() == 0);
            REQUIRE(a1o.allocated() == 0);

            /* arbitrary alloc size */
            size_t req_z = 13;
            byte * mem = a1o.alloc(typeseq::sentinel(), req_z);

            REQUIRE(arena.error_count_ == 0);
            REQUIRE(mem != nullptr);

            {
                DArenaIterator     ix = arena.begin();
                DArenaIterator end_ix = arena.end();

                REQUIRE(ix.is_valid());
                REQUIRE(end_ix.is_valid());
                /* arena is non-empty, so begin!=end */
                REQUIRE (ix != end_ix);

                REQUIRE(arena.error_count_ == 0);

                /* valid iterator can be dereferenced */
                {
                    AllocInfo info = *ix;

                    REQUIRE(arena.error_count_ == 0);
                    REQUIRE(info.is_valid());
                    REQUIRE(info.size() == padding::with_padding(req_z));

                    auto [payload_lo, payload_hi] = info.payload();

                    REQUIRE(payload_lo == mem);
                    REQUIRE(payload_hi == mem + info.size());
                }

                /* valid iterator can be advanced */
                {
                    ix.next();

                    REQUIRE(arena.error_count_ == 0);
                    REQUIRE(ix == end_ix);
                }
            }

            // repeat, this time with generic iterators
            {
                log && log(xtag("section", "obj<AAllocIterator>"),
                           xtag("arena", &arena),
                           xtag("arena.lo", arena.lo_),
                           xtag("arena.free", arena.free_));

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

                log && log(xtag("ix._typeseq", ix._typeseq()),
                           xtag("ix.data", ix.data()));

                log && log(xtag("typeseq<DArena>",
                                typeseq::id<DArena>()));
                log && log(xtag("typeseq<DArenaIterator>",
                                typeseq::id<DArenaIterator>()));
                log && log(xtag("typeseq<DVariantPlaceholder>",
                                typeseq::id<DVariantPlaceholder>()));

                REQUIRE(ix.compare(ix).is_equal());

                //REQUIRE(ix.compare(end_ix).is_equal());

                REQUIRE(ix != end_ix);

                {
                    REQUIRE(arena.error_count_ == 0);
                    REQUIRE(ix.deref().is_valid());
                    REQUIRE(ix.deref().size() == padding::with_padding(req_z));

                    auto [payload_lo, payload_hi] = ix.deref().payload();

                    REQUIRE(payload_lo == mem);
                    REQUIRE(payload_hi == mem + ix.deref().size());
                }

                /* valid iterator can be advanced + reaches end */
                {
                    ++ix; //ix.next();

                    REQUIRE(arena.error_count_ == 0);
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

/* end AllocIterator.test.cpp */
