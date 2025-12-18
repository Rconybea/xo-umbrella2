/** @file AllocIterator.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "Allocator.hpp"
#include "AllocIterator.hpp"
#include "arena/IAllocator_DArena.hpp"
#include "arena/IAllocIterator_DArenaIterator.hpp"
#include "padding.hpp"
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

            /* iteration not supported since we did not set */

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

            byte * mem = a1o.alloc(req_z);
            REQUIRE(arena.error_count_ == 0);
            REQUIRE(mem != nullptr);

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

    } /*namespace ut*/
} /*namespace xo*/

/* end AllocIterator.test.cpp */
