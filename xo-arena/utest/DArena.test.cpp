/** @file DArena.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "xo/arena/DArena.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
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

    }
}

/* end DArena.test.cpp */
