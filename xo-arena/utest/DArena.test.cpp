/** @file DArena.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArena.hpp"
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/tag_ostream.hpp> /* Catch2 INFO() streams the tag to an ostream */
#include <catch2/catch.hpp>
#include <cstdint>
#include <utility>

namespace xo {
    using xo::mm::DArena;
    using xo::mm::AllocHeader;
    using xo::mm::AllocHeaderConfig;
    using xo::mm::ArenaConfig;
    using xo::mm::ArenaNameStr;
    using xo::mm::padding;
    using xo::mm::error;
    using xo::reflect::typeseq;
    using xo::reflect::typeseq;
    using xo::pp::xtag;
    using std::byte;

    namespace ut {
        TEST_CASE("DArena-tiny", "[arena][DArena]")
        {
            ArenaConfig cfg { .name_ = ArenaNameStr::from_chars("testarena"),
                              .size_ = 1 };
            DArena arena = DArena::map(cfg);

            REQUIRE(arena.config().name_ == cfg.name_);
            REQUIRE(arena._mem_lo() != nullptr);
            REQUIRE(arena.free_ == arena._mem_lo());
            REQUIRE(arena.limit_ == arena._mem_lo());
            REQUIRE(arena.hi_ != nullptr);
            REQUIRE(arena.hi_ > arena._mem_lo());
            REQUIRE(((size_t)arena.hi_ - (size_t)arena._mem_lo()) % arena.page_z_ == 0);
            REQUIRE(arena._mem_lo() + cfg.size_ <= arena.hi_);

            /* verify arena._mem_lo() is aligned on a page boundary */
            REQUIRE(((size_t)(arena._mem_lo()) & (arena.page_z_ - 1)) == 0);

            /* verify arena.hi_ is aligned on a hugepage boundary */
            REQUIRE(((size_t)(arena.hi_) & (arena.page_z_ - 1)) == 0);

            byte * lo = arena._mem_lo();
            byte * free = arena.free_;
            byte * limit = arena.limit_;
            byte * hi = arena.hi_;
            size_t committed_z = arena.committed_z_;

            DArena arena2 = std::move(arena);

            REQUIRE(arena._mem_lo() == nullptr);
            REQUIRE(arena.free_ == nullptr);
            REQUIRE(arena.limit_ == nullptr);
            REQUIRE(arena.hi_ == nullptr);
            REQUIRE(arena.committed_z_ == 0);

            REQUIRE(arena._mem_lo() == nullptr);
            REQUIRE(arena2._mem_lo() == lo);
            REQUIRE(arena2.free_ == free);
            REQUIRE(arena2.limit_ == limit);
            REQUIRE(arena2.hi_ == hi);
            REQUIRE(arena2.committed_z_ == committed_z);
        }

        TEST_CASE("DArena-medium", "[arena][DArena]")
        {
            ArenaConfig cfg { .name_ = ArenaNameStr::from_chars("testarena"),
                              .size_ = 10*1024*1024 };
            DArena arena = DArena::map(cfg);

            REQUIRE(arena.config().name_ == cfg.name_);
            REQUIRE(arena._mem_lo() != nullptr);
            REQUIRE(arena.free_ == arena._mem_lo());
            REQUIRE(arena.limit_ == arena._mem_lo());
            REQUIRE(arena.hi_ != nullptr);
            REQUIRE(arena.hi_ > arena._mem_lo());
            REQUIRE(((size_t)arena.hi_ - (size_t)arena._mem_lo()) % cfg.hugepage_z_ == 0);
            REQUIRE(arena._mem_lo() + cfg.size_ <= arena.hi_);

            /* verify arena._mem_lo() is aligned on a page boundary */
            REQUIRE(((size_t)(arena._mem_lo()) & (cfg.hugepage_z_ - 1)) == 0);

            /* verify arena.hi_ is aligned on a hugepage boundary */
            REQUIRE(((size_t)(arena.hi_) & (cfg.hugepage_z_ - 1)) == 0);

            byte * lo = arena._mem_lo();
            byte * free = arena.free_;
            byte * limit = arena.limit_;
            byte * hi = arena.hi_;
            size_t committed_z = arena.committed_z_;

            DArena arena2 = std::move(arena);

            REQUIRE(arena._mem_lo() == nullptr);
            REQUIRE(arena.free_ == nullptr);
            REQUIRE(arena.limit_ == nullptr);
            REQUIRE(arena.hi_ == nullptr);
            REQUIRE(arena.committed_z_ == 0);

            REQUIRE(arena._mem_lo() == nullptr);
            REQUIRE(arena2._mem_lo() == lo);
            REQUIRE(arena2.free_ == free);
            REQUIRE(arena2.limit_ == limit);
            REQUIRE(arena2.hi_ == hi);
            REQUIRE(arena2.committed_z_ == committed_z);
        }

        TEST_CASE("DArena-expand-1", "[arena][DArena]")
        {
            /* typed allocator a1o */
            ArenaConfig cfg { .name_ = ArenaNameStr::from_cstr("testarena"),
                              .size_ = 1,
                              .debug_flag_ = false };
            DArena arena = DArena::map(cfg);

            REQUIRE(arena.available() == 0);
            REQUIRE(arena.allocated() == 0);

            size_t z2 = 512;
            bool ok = arena.expand(z2, __PRETTY_FUNCTION__);

            INFO(xtag("last_error", arena.last_error()));

            REQUIRE(ok);

            REQUIRE(arena.reserved() % arena.page_z() == 0);
            REQUIRE(arena.committed() >= z2);
            REQUIRE(arena.committed() % arena.page_z() == 0);
            REQUIRE(arena.available() >= z2);
            REQUIRE(arena.preamble_z() + arena.available() == arena.committed());
            REQUIRE(arena.allocated() == arena.preamble_z());
        }

        TEST_CASE("arena-alloc-1", "[arena][DArena]")
        {
            /* typed allocator a1o */
            ArenaConfig cfg { .name_ = ArenaNameStr::from_cstr("testarena"),
                              .size_ = 64*1024,
                              .debug_flag_ = false };
            DArena arena = DArena::map(cfg);

            REQUIRE(arena.reserved() >= cfg.size_);
            REQUIRE(arena.committed() == 0);
            REQUIRE(arena.available() == 0);
            REQUIRE(arena.allocated() == 0);

            size_t z0 = 1;
            byte * m0 = arena.alloc(typeseq::sentinel(), 1);

            REQUIRE(m0);
            REQUIRE(arena.last_error().error_ == error::ok);
            REQUIRE(arena.last_error().error_seq_ == 0);
            REQUIRE(arena.allocated() >= z0);
            REQUIRE(arena.allocated() < arena.preamble_z() + z0 + padding::c_alloc_alignment );
            REQUIRE(arena.allocated() <= arena.committed());
            REQUIRE(arena.allocated() + arena.available() == arena.committed());
            REQUIRE(arena.committed() <= arena.reserved());

            size_t z1 = 16;
            byte * m1 = arena.alloc(typeseq::sentinel(), z1);

            REQUIRE(m1);
            REQUIRE(arena.last_error().error_ == error::ok);
            REQUIRE(arena.last_error().error_seq_ == 0);
            REQUIRE(arena.allocated() >= z0 + z1);
            REQUIRE(arena.allocated() < z0 + z1 + 2 * padding::c_alloc_alignment );
            REQUIRE(arena.allocated() <= arena.committed());
            REQUIRE(arena.allocated() + arena.available() == arena.committed());
            REQUIRE(arena.committed() <= arena.reserved());
        }

        TEST_CASE("arena-alloc-2", "[arena][DArena]")
        {
            using header_type = AllocHeader;

            /* typed allocator a1o, with object header */
            ArenaConfig cfg { .name_ = ArenaNameStr::from_cstr("testarena"),
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

            REQUIRE(arena.reserved() >= cfg.size_);
            REQUIRE(arena.committed() == 0);
            REQUIRE(arena.available() == 0);
            REQUIRE(arena.allocated() == 0);

            size_t z0 = 1;
            byte * m0 = arena.alloc(typeseq::sentinel(), 1);

            REQUIRE(m0);

            header_type* header = (header_type*)(m0 - sizeof(header_type));

            REQUIRE(arena.contains(header));
            REQUIRE(cfg.header_.size(*header) == padding::with_padding(z0));
            //REQUIRE(((*header) & cfg.header_size_mask_) == padding::with_padding(z0));
            REQUIRE(arena.last_error().error_ == error::ok);
            REQUIRE(arena.last_error().error_seq_ == 0);
            REQUIRE(arena.allocated() >= z0);
            REQUIRE(arena.allocated() < arena.preamble_z() + arena.per_alloc_overhead_z() + z0 + padding::c_alloc_alignment );
            REQUIRE(arena.allocated() <= arena.committed());
            REQUIRE(arena.allocated() + arena.available() == arena.committed());
            REQUIRE(arena.committed() <= arena.reserved());
        }

        namespace {
            /** @p align must be a power of two; masks off its low-order bits **/
            std::uintptr_t mask_to_block(const void * p, std::size_t align) {
                return reinterpret_cast<std::uintptr_t>(p) & ~(static_cast<std::uintptr_t>(align) - 1);
            }

            std::pair<std::uintptr_t, std::uintptr_t> bounds(const DArena & a) {
                std::uintptr_t lo = 0, hi = 0;
                a.visit_pools([&lo, &hi](const xo::mm::MemorySizeInfo & m) {
                        lo = reinterpret_cast<std::uintptr_t>(m.lo_);
                        hi = reinterpret_cast<std::uintptr_t>(m.hi_);
                    });
                return {lo, hi};
            }

            ArenaConfig aligned_cfg(std::size_t size_z, std::size_t align_z, bool exclusive) {
                return ArenaConfig()
                    .with_name(ArenaNameStr::from_chars("utest.align"))
                    .with_size(size_z)
                    .with_base_align_z(align_z)
                    .with_exclusive_block_flag(exclusive);
            }
        }

        TEST_CASE("darena-base-align-is-maskable", "[arena][base_align]")
        {
            /* 2GB alignment with a 1MB arena: the interesting case, because the
             * extent and the alignment are wildly different.  Reserving address
             * space is cheap -- measured 2026-09-17, a 2GB PROT_NONE mapping
             * adds ZERO page-table entries; they appear on first touch.
             */
            constexpr std::size_t c_align = 2UL * 1024 * 1024 * 1024;
            constexpr std::size_t c_size = 1UL * 1024 * 1024;

            DArena arena = DArena::map(aligned_cfg(c_size, c_align, false));

            auto [lo, hi] = bounds(arena);

            REQUIRE(lo % c_align == 0);

            /* the extent is what was ASKED for, not the block.  Rounding the
             * extent up to the base alignment would reserve 2GB here, and that
             * is what with_exclusive_block_flag is for -- it must not happen by
             * accident
             */
            REQUIRE(hi - lo == c_size);

            /* the property the feature exists for */
            auto * m0 = arena.alloc(typeseq::sentinel(), 64);
            auto * m1 = arena.alloc(typeseq::sentinel(), 64);

            REQUIRE(m0);
            REQUIRE(m1);
            REQUIRE(mask_to_block(m0, c_align) == lo);
            REQUIRE(mask_to_block(m1, c_align) == lo);

            /* committing must not round up to the BASE alignment.  When it did,
             * a 64-byte alloc tried to commit 2GB: mprotect failed for a 1MB
             * arena, and for an exclusive one it silently committed the whole
             * block.  arena_align_z_ is the COMMIT granularity, not the base
             * alignment -- see DArena::map.
             */
            REQUIRE(arena.committed() < c_size);
            REQUIRE(arena.committed() <= arena.reserved());
        }

        TEST_CASE("darena-exclusive-block-claims-the-whole-block", "[arena][base_align]")
        {
            constexpr std::size_t c_align = 2UL * 1024 * 1024 * 1024;
            constexpr std::size_t c_size = 1UL * 1024 * 1024;

            DArena arena = DArena::map(aligned_cfg(c_size, c_align, true));

            auto [lo, hi] = bounds(arena);

            REQUIRE(lo % c_align == 0);
            /* the whole block, so nothing unrelated can be mapped in it and
             * mask to this arena's base
             */
            REQUIRE(hi - lo == c_align);

            auto * m0 = arena.alloc(typeseq::sentinel(), 64);

            REQUIRE(m0);
            REQUIRE(mask_to_block(m0, c_align) == lo);
            REQUIRE(arena.committed() < c_size);
        }

        TEST_CASE("darena-obj2arena-recovers-the-arena", "[arena][base_align][obj2arena]")
        {
            /* the payoff of base alignment: from an interior pointer alone,
             * recover the DArena that owns it -- no lookup, no registry
             */
            constexpr std::size_t c_align = 2UL * 1024 * 1024 * 1024;
            constexpr std::size_t c_size = 1UL * 1024 * 1024;

            DArena arena = DArena::map(aligned_cfg(c_size, c_align, true));

            auto * m0 = arena.alloc(typeseq::sentinel(), 64);
            auto * m1 = arena.alloc(typeseq::sentinel(), 64);

            REQUIRE(m0);
            REQUIRE(m1);

            REQUIRE(DArena::obj2arena(m0, c_align) == &arena);
            REQUIRE(DArena::obj2arena(m1, c_align) == &arena);

            /* interior of an allocation, not just its first byte */
            REQUIRE(DArena::obj2arena(m0 + 17, c_align) == &arena);

            /* the mask must clear the LOW bits, not keep them.  `n & (align-1)'
             * yields the offset WITHIN the block -- 0x10 for the first
             * allocation -- which then gets dereferenced.  Asserting the
             * recovered pointer is the arena would pass for a one-arena
             * process by luck if the wrong expression happened not to fault,
             * so pin the arithmetic itself.
             */
            auto n0 = reinterpret_cast<std::uintptr_t>(m0);

            REQUIRE((n0 & ~(static_cast<std::uintptr_t>(c_align) - 1)) == bounds(arena).first);
            REQUIRE((n0 & (static_cast<std::uintptr_t>(c_align) - 1)) != bounds(arena).first);
        } /*TEST_CASE(darena-obj2arena-recovers-the-arena)*/

        TEST_CASE("darena-obj2arena-across-two-arenas", "[arena][base_align][obj2arena]")
        {
            /* the case a single arena cannot exercise: two participating
             * arenas sharing one alignment policy, each pointer resolving to
             * its OWN arena.  A mask that returned a constant, or that read the
             * wrong slot, passes the single-arena test and fails here.
             */
            constexpr std::size_t c_align = 2UL * 1024 * 1024 * 1024;
            constexpr std::size_t c_size = 1UL * 1024 * 1024;

            DArena a1 = DArena::map(aligned_cfg(c_size, c_align, true));
            DArena a2 = DArena::map(aligned_cfg(c_size, c_align, true));

            REQUIRE(bounds(a1).first != bounds(a2).first);

            auto * p1 = a1.alloc(typeseq::sentinel(), 64);
            auto * p2 = a2.alloc(typeseq::sentinel(), 64);

            REQUIRE(DArena::obj2arena(p1, c_align) == &a1);
            REQUIRE(DArena::obj2arena(p2, c_align) == &a2);
        } /*TEST_CASE(darena-obj2arena-across-two-arenas)*/

        TEST_CASE("darena-obj2arena-survives-a-move", "[arena][base_align][obj2arena]")
        {
            /* the back pointer names whichever DArena currently owns the
             * storage, so a move must re-point it -- that is what
             * fixup_meta_pointer is for.  Without it obj2arena hands back a
             * stale DArena* that no longer owns anything.
             */
            constexpr std::size_t c_align = 2UL * 1024 * 1024 * 1024;
            constexpr std::size_t c_size = 1UL * 1024 * 1024;

            DArena a1 = DArena::map(aligned_cfg(c_size, c_align, true));
            auto * p = a1.alloc(typeseq::sentinel(), 64);

            REQUIRE(DArena::obj2arena(p, c_align) == &a1);

            DArena a2 = std::move(a1);

            REQUIRE(DArena::obj2arena(p, c_align) == &a2);
        } /*TEST_CASE(darena-obj2arena-survives-a-move)*/

        TEST_CASE("darena-base-align-rejects-what-cannot-work", "[arena][base_align]")
        {
            /* an arena larger than its alignment spans two blocks, so a pointer
             * in the second masks to the wrong base.  Refused rather than left
             * to give a wrong answer later.
             */
            REQUIRE_THROWS(DArena::map(aligned_cfg(4 * 1024 * 1024,
                                                   1024 * 1024,
                                                   false)));

            /* masking is meaningless unless the alignment is a power of two */
            REQUIRE_THROWS(DArena::map(aligned_cfg(1024, 3 * 4096, false)));
        }
    }
}

/* end DArena.test.cpp */
