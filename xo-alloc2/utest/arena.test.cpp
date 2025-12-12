/** @file arena.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "xo/alloc2/AAllocator.hpp"
#include "xo/alloc2/IAllocator_Any.hpp"
#include "xo/alloc2/IAllocator_Xfer.hpp"
//#include "xo/alloc2/DArena.hpp"
#include "xo/alloc2/IAllocator_DArena.hpp"
#include "xo/alloc2/RAllocator.hpp"
#include "xo/alloc2/padding.hpp"
#include "xo/facet/obj.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::AAllocator;
    using xo::mm::IAllocator_DArena;
    using xo::mm::IAllocator_Xfer;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::obj;
    using std::byte;
    using std::size_t;

    namespace ut {
        TEST_CASE("IAllocator_Xfer_DArena", "[alloc2]")
        {
            IAllocator_Xfer<DArena, IAllocator_DArena> xfer;

            REQUIRE(IAllocator_Xfer<DArena, IAllocator_DArena>::_valid);
        }

        TEST_CASE("DArena", "[alloc2][DArena]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 1 };
            DArena arena = DArena::map(cfg);

            REQUIRE(arena.config_.name_ == cfg.name_);
            REQUIRE(arena.lo_ != nullptr);
            REQUIRE(arena.free_ == arena.lo_);
            REQUIRE(arena.limit_ == arena.lo_);
            REQUIRE(arena.hi_ != nullptr);
            REQUIRE(arena.hi_ > arena.lo_);
            REQUIRE(((size_t)arena.hi_ - (size_t)arena.lo_) % cfg.hugepage_z_ == 0);
            REQUIRE(arena.lo_ + cfg.size_ <= arena.hi_);

            /* verify arena.lo_ is aligned on a hugepage boundary */
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
            obj<AAllocator, DArena> a1o{&arena};

            REQUIRE(a1o);
            REQUIRE(a1o.iface() != nullptr);
            REQUIRE(a1o.data() != nullptr);

            REQUIRE(a1o._typeseq()
                    == xo::facet::FacetImplType<AAllocator, DArena>::s_typeseq);
            REQUIRE(a1o.name() == cfg.name_);
            REQUIRE(a1o.reserved() >= cfg.size_);
            REQUIRE(a1o.reserved() < cfg.size_ + cfg.hugepage_z_);
            REQUIRE(a1o.reserved() % cfg.hugepage_z_ == 0);
            REQUIRE(a1o.size() == 0);
            REQUIRE(a1o.committed() == 0);
        }

        TEST_CASE("allocator-expand-1", "[alloc2][AAllocator]")
        {
            /* typed allocator a1o */
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 1 };
            DArena arena = DArena::map(cfg);
            obj<AAllocator, DArena> a1o{&arena};

            a1o.expand(3*1024*1024);

            REQUIRE(a1o.reserved() % cfg.hugepage_z_ == 0);

#ifdef NOPE
            byte * m = a1o.alloc(1);

            REQUIRE(m);
#endif
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end arena.test.cpp */
