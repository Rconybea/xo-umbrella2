/** @file arena.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "xo/alloc2/Allocator.hpp"
#include "xo/alloc2/alloc/IAllocator_Xfer.hpp"
#include "xo/alloc2/arena/IAllocator_DArena.hpp"
#include "xo/arena/print.hpp"
#include "xo/arena/padding.hpp"
#include <xo/facet/obj.hpp>
#include <xo/indentlog/scope.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::AAllocator;
    using xo::mm::IAllocator_DArena;
    using xo::mm::IAllocator_Xfer;
    using xo::mm::AllocError;
    using xo::mm::DArena;
    using xo::mm::AllocHeaderConfig;
    using xo::mm::ArenaConfig;
    using xo::mm::AllocHeader;
    using xo::mm::padding;
    using xo::mm::error;
    using xo::facet::with_facet;
    using xo::facet::obj;
    using xo::facet::typeseq;
    using xo::scope;
    using std::byte;
    using std::size_t;

    namespace ut {
        TEST_CASE("IAllocator_Xfer_DArena", "[alloc2]")
        {
            IAllocator_Xfer<DArena, IAllocator_DArena> xfer;

            REQUIRE(IAllocator_Xfer<DArena, IAllocator_DArena>::_valid);
        }

        TEST_CASE("DArena-medium", "[alloc2][DArena]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 10*1024*1024 };
            DArena arena = DArena::map(cfg);

            REQUIRE(arena.config_.name_ == cfg.name_);
            REQUIRE(arena.lo_ != nullptr);
            REQUIRE(arena.free_ == arena.lo_);
            REQUIRE(arena.limit_ == arena.lo_);
            REQUIRE(arena.hi_ != nullptr);
            REQUIRE(arena.hi_ > arena.lo_);
            REQUIRE(((size_t)arena.hi_ - (size_t)arena.lo_) % cfg.hugepage_z_ == 0);
            REQUIRE(arena.lo_ + cfg.size_ <= arena.hi_);

            /* verify arena.lo_ is aligned on a page boundary */
            REQUIRE(((size_t)(arena.lo_) & (cfg.hugepage_z_ - 1)) == 0);

            /* verify arena.hi_ is aligned on a hugepage boundary */
            REQUIRE(((size_t)(arena.hi_) & (cfg.hugepage_z_ - 1)) == 0);

            byte * lo = arena.lo_;
            byte * free = arena.free_;
            byte * limit = arena.limit_;
            byte * hi = arena.hi_;
            size_t committed_z = arena.committed_z_;

            DArena arena2 = std::move(arena);

            REQUIRE(arena.lo_ == nullptr);
            REQUIRE(arena.free_ == nullptr);
            REQUIRE(arena.limit_ == nullptr);
            REQUIRE(arena.hi_ == nullptr);
            REQUIRE(arena.committed_z_ == 0);

            REQUIRE(arena.lo_ == nullptr);
            REQUIRE(arena2.lo_ == lo);
            REQUIRE(arena2.free_ == free);
            REQUIRE(arena2.limit_ == limit);
            REQUIRE(arena2.hi_ == hi);
            REQUIRE(arena2.committed_z_ == committed_z);
        }

        TEST_CASE("allocator-any-1", "[alloc2][AAllocator]")
        {
            /* empty allocator alloc1 */
            obj<AAllocator> alloc1;

            REQUIRE(!alloc1);
            REQUIRE(alloc1.iface() != nullptr);
            REQUIRE(alloc1.data() == nullptr);

            /* typed allocator a1o */
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 1 };
            DArena arena = DArena::map(cfg);
            //obj<AAllocator, DArena> a1o{&arena};
            auto a1o = with_facet<AAllocator>::mkobj(&arena);

            REQUIRE(a1o);
            REQUIRE(a1o.iface() != nullptr);
            REQUIRE(a1o.data() != nullptr);

            REQUIRE(a1o._typeseq()
                    == xo::facet::FacetImplType<AAllocator, DArena>::s_typeseq);
            REQUIRE(a1o.name() == cfg.name_);
            REQUIRE(a1o.reserved() >= cfg.size_);
            REQUIRE(a1o.reserved() < cfg.size_ + a1o.data()->page_z_);
            REQUIRE(a1o.reserved() % a1o.data()->page_z_ == 0);
            REQUIRE(a1o.size() == 0);
            REQUIRE(a1o.committed() == 0);
            REQUIRE(a1o.allocated() == 0);
        }

        TEST_CASE("allocator-expand-1", "[alloc2][AAllocator]")
        {
            /* typed allocator a1o */
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 1,
                              .debug_flag_ = false };
            DArena arena = DArena::map(cfg);
            //obj<AAllocator, DArena> a1o{&arena};
            auto a1o = with_facet<AAllocator>::mkobj(&arena);

            REQUIRE(a1o.available() == 0);
            REQUIRE(a1o.allocated() == 0);

            size_t z2 = 512;
            bool ok = a1o.expand(z2);

            INFO(xtag("last_error", a1o.last_error()));

            REQUIRE(ok);

            REQUIRE(a1o.reserved() % a1o.data()->page_z_ == 0);
            REQUIRE(a1o.committed() >= z2);
            REQUIRE(a1o.committed() % a1o.data()->page_z_ == 0);
            /* .size() is synonym for .committed() */
            REQUIRE(a1o.size() == a1o.committed());
            REQUIRE(a1o.available() >= z2);
            REQUIRE(a1o.available() == a1o.committed());
            REQUIRE(a1o.allocated() == 0);

        }

        TEST_CASE("allocator-alloc-1", "[alloc2][AAllocator]")
        {
            /* typed allocator a1o */
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 64*1024,
                              .debug_flag_ = false };
            DArena arena = DArena::map(cfg);
            //obj<AAllocator, DArena> a1o{&arena};
            auto a1o = with_facet<AAllocator>::mkobj(&arena);

            REQUIRE(a1o.reserved() >= cfg.size_);
            REQUIRE(a1o.committed() == 0);
            REQUIRE(a1o.available() == 0);
            REQUIRE(a1o.allocated() == 0);

            size_t z0 = 1;
            byte * m0 = a1o.alloc(typeseq::sentinel(), 1);

            REQUIRE(m0);
            REQUIRE(a1o.last_error().error_ == error::ok);
            REQUIRE(a1o.last_error().error_seq_ == 0);
            REQUIRE(a1o.allocated() >= z0);
            REQUIRE(a1o.allocated() < z0 + padding::c_alloc_alignment );
            REQUIRE(a1o.allocated() <= a1o.committed());
            REQUIRE(a1o.allocated() + a1o.available() == a1o.committed());
            REQUIRE(a1o.committed() <= a1o.reserved());

            size_t z1 = 16;
            byte * m1 = a1o.alloc(typeseq::sentinel(), z1);

            REQUIRE(m1);
            REQUIRE(a1o.last_error().error_ == error::ok);
            REQUIRE(a1o.last_error().error_seq_ == 0);
            REQUIRE(a1o.allocated() >= z0 + z1);
            REQUIRE(a1o.allocated() < z0 + z1 + 2 * padding::c_alloc_alignment );
            REQUIRE(a1o.allocated() <= a1o.committed());
            REQUIRE(a1o.allocated() + a1o.available() == a1o.committed());
            REQUIRE(a1o.committed() <= a1o.reserved());
        }

        TEST_CASE("allocator-alloc-2", "[alloc2][Allocator]")
        {
            using header_type = AllocHeader;

            /* typed allocator a1o, with object header */
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 64*1024,
                              .store_header_flag_ = true,
                              /* up to 4GB */
                              .header_ = AllocHeaderConfig(0 /*guard_z*/,
                                                           0xfd /*guard_byte*/,
                                                           0 /*tseq-bits*/,
                                                           0 /*age-bits*/,
                                                           32 /*size-bits*/),
                              .debug_flag_ = false,
            };
            DArena arena = DArena::map(cfg);
            //obj<AAllocator, DArena> a1o{&arena};
            auto a1o = with_facet<AAllocator>::mkobj(&arena);

            REQUIRE(a1o.reserved() >= cfg.size_);
            REQUIRE(a1o.committed() == 0);
            REQUIRE(a1o.available() == 0);
            REQUIRE(a1o.allocated() == 0);

            size_t z0 = 1;
            byte * m0 = a1o.alloc(typeseq::sentinel(), 1);

            REQUIRE(m0);

            header_type* header = (header_type*)(m0 - sizeof(header_type));

            REQUIRE(a1o.contains(header));
            REQUIRE(cfg.header_.size(*header) == padding::with_padding(z0));
            //REQUIRE(((*header) & cfg.header_size_mask_) == padding::with_padding(z0));
            REQUIRE(a1o.last_error().error_ == error::ok);
            REQUIRE(a1o.last_error().error_seq_ == 0);
            REQUIRE(a1o.allocated() >= z0);
            REQUIRE(a1o.allocated() < sizeof(AAllocator::header_type) + z0 + padding::c_alloc_alignment );
            REQUIRE(a1o.allocated() <= a1o.committed());
            REQUIRE(a1o.allocated() + a1o.available() == a1o.committed());
            REQUIRE(a1o.committed() <= a1o.reserved());
        }

        TEST_CASE("allocator-alloc-3", "[alloc2][Allocator]")
        {
            using header_type = AllocHeader;

            /* typed allocator a1o, with object header + guard bytes */
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 64*1024,
                              .store_header_flag_ = true,
                              /* up to 4GB */
                              .header_ = AllocHeaderConfig(8 /*guard_z*/,
                                                           0xfd /*guard-byte*/,
                                                           0 /*tseq-bits*/,
                                                           0 /*age-bits*/,
                                                           32 /*size-bits*/),
                              .debug_flag_ = false,
            };
            DArena arena = DArena::map(cfg);
            //obj<AAllocator, DArena> a1o{&arena};
            auto a1o = with_facet<AAllocator>::mkobj(&arena);

            REQUIRE(a1o.reserved() >= cfg.size_);
            REQUIRE(a1o.committed() == 0);
            REQUIRE(a1o.available() == 0);
            REQUIRE(a1o.allocated() == 0);

            size_t z0 = 1;
            byte * m0 = a1o.alloc(typeseq::sentinel(), 1);

            REQUIRE(m0);

            //
            //                > <
            //  < guard><header> < pad >< guard>
            //  ++++++++0000zzzzXppppppp++++++++
            //  ^       ^       ^       ^
            //  guard0  header  m0      guard1
            //

            byte * guard0 = m0 - sizeof(header_type) - cfg.header_.guard_z_;
            header_type* header = (header_type*)(m0 - sizeof(header_type));
            size_t pad = padding::with_padding(z0) - z0;
            byte * guard1 = m0 + z0 + pad;

            REQUIRE(a1o.contains(guard0));
            REQUIRE(a1o.contains(header));
            REQUIRE(cfg.header_.size(*header) == padding::with_padding(z0));
            //REQUIRE(((*header) & cfg.header_size_mask_) == padding::with_padding(z0));

            REQUIRE(a1o.last_error().error_ == error::ok);
            REQUIRE(a1o.last_error().error_seq_ == 0);

            REQUIRE(a1o.allocated() == (cfg.header_.guard_z_
                                        + sizeof(header_type)
                                        + z0
                                        + pad
                                        + cfg.header_.guard_z_));
            REQUIRE(a1o.allocated() <= a1o.committed());
            REQUIRE(a1o.allocated() + a1o.available() == a1o.committed());
            REQUIRE(a1o.committed() <= a1o.reserved());
        }

        TEST_CASE("allocator-fail-1", "[alloc2][AAllocator]")
        {
            /* typed allocator a1o */
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 64*1024,
                              .debug_flag_ = false };
            DArena arena = DArena::map(cfg);
            auto a1o = with_facet<AAllocator>::mkobj(&arena);
            //obj<AAllocator, DArena> a1o{&arena};

            REQUIRE(cfg.size_ <= cfg.hugepage_z_);

            REQUIRE(a1o.reserved() >= cfg.size_);
            REQUIRE(a1o.committed() == 0);
            REQUIRE(a1o.available() == 0);
            REQUIRE(a1o.allocated() == 0);

            size_t z0 = cfg.hugepage_z_ + 1;
            byte * m0 = a1o.alloc(typeseq::sentinel(), z0);

            REQUIRE(!m0);

            AllocError err = a1o.last_error();

            REQUIRE(err.error_ == error::reserve_exhausted);
            REQUIRE(err.error_seq_ == 1);
            REQUIRE(err.request_z_ >= z0);
            REQUIRE(err.request_z_ < z0 + padding::c_alloc_alignment);
            REQUIRE(err.committed_z_ == 0);
            REQUIRE(err.reserved_z_ == arena.reserved());
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end arena.test.cpp */
