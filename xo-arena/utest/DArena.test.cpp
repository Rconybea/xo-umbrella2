/** @file DArena.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArena.hpp"
#include "print.hpp"
#include <xo/indentlog/print/tag.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::mm::padding;
    using xo::mm::error;
    using xo::reflect::typeseq;
    using xo::xtag;
    using std::byte;

    namespace ut {
        TEST_CASE("DArena-tiny", "[arena][DArena]")
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
            REQUIRE(((size_t)arena.hi_ - (size_t)arena.lo_) % arena.page_z_ == 0);
            REQUIRE(arena.lo_ + cfg.size_ <= arena.hi_);

            /* verify arena.lo_ is aligned on a page boundary */
            REQUIRE(((size_t)(arena.lo_) & (arena.page_z_ - 1)) == 0);

            /* verify arena.hi_ is aligned on a hugepage boundary */
            REQUIRE(((size_t)(arena.hi_) & (arena.page_z_ - 1)) == 0);

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

        TEST_CASE("DArena-medium", "[arena][DArena]")
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

        TEST_CASE("DArena-expand-1", "[arena][DArena]")
        {
            /* typed allocator a1o */
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 1,
                              .debug_flag_ = false };
            DArena arena = DArena::map(cfg);

            REQUIRE(arena.available() == 0);
            REQUIRE(arena.allocated() == 0);

            size_t z2 = 512;
            bool ok = arena.expand(z2);

            INFO(xtag("last_error", arena.last_error()));

            REQUIRE(ok);

            REQUIRE(arena.reserved() % arena.page_z() == 0);
            REQUIRE(arena.committed() >= z2);
            REQUIRE(arena.committed() % arena.page_z() == 0);
            REQUIRE(arena.available() >= z2);
            REQUIRE(arena.available() == arena.committed());
            REQUIRE(arena.allocated() == 0);

        }

        TEST_CASE("arena-alloc-1", "[arena][DArena]")
        {
            /* typed allocator a1o */
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 64*1024,
                              .debug_flag_ = false };
            DArena arena = DArena::map(cfg);

            REQUIRE(arena.reserved() >= cfg.size_);
            REQUIRE(arena.committed() == 0);
            REQUIRE(arena.available() == 0);
            REQUIRE(arena.allocated() == 0);

            size_t z0 = 1;
            byte * m0 = arena.alloc(typeseq::anon(), 1);

            REQUIRE(m0);
            REQUIRE(arena.last_error().error_ == error::ok);
            REQUIRE(arena.last_error().error_seq_ == 0);
            REQUIRE(arena.allocated() >= z0);
            REQUIRE(arena.allocated() < z0 + padding::c_alloc_alignment );
            REQUIRE(arena.allocated() <= arena.committed());
            REQUIRE(arena.allocated() + arena.available() == arena.committed());
            REQUIRE(arena.committed() <= arena.reserved());

            size_t z1 = 16;
            byte * m1 = arena.alloc(typeseq::anon(), z1);

            REQUIRE(m1);
            REQUIRE(arena.last_error().error_ == error::ok);
            REQUIRE(arena.last_error().error_seq_ == 0);
            REQUIRE(arena.allocated() >= z0 + z1);
            REQUIRE(arena.allocated() < z0 + z1 + 2 * padding::c_alloc_alignment );
            REQUIRE(arena.allocated() <= arena.committed());
            REQUIRE(arena.allocated() + arena.available() == arena.committed());
            REQUIRE(arena.committed() <= arena.reserved());
        }

    }
}

/* end DArena.test.cpp */
