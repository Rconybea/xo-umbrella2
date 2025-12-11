/** @file DArena.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "xo/alloc2/AAllocator.hpp"
#include "xo/alloc2/DArena.hpp"
#include "xo/alloc2/padding.hpp"
#include <cassert>
#include <sys/mman.h> // for ::munmap()
#include <unistd.h> // for ::getpagesize()

namespace xo {
    using std::byte;

    namespace mm {

        DArena::DArena(const ArenaConfig & cfg,
                       std::byte * lo,
                       std::byte * hi
            )
        {
            //scope log(XO_DEBUG(debug_flag), xtag("name", name));

            this->page_z_     = getpagesize();

            // 1. need k pagetable entries where k is lub {k | k * .page_z >= z}
            // 2. base will be aligned with .page_z but likely not with .hugepage_z
            // 3. bad to have misalignment, because misaligned {prefix, suffix} of [base, base+z)
            //    will use 4k pages instead of 2mb pages
            //
            // strategy:
            // 4. round up z to multiple of hugepage_z_
            // 5. over-request so reserved range contains an aligned subrange of size z
            // 6. unmap misaligned prefix
            // 7. unmap misaligned suffix.
            // 8. enable huge pages for now-aligned remainder of reserved range
            //
            // Z. note: rejecting inferior MAP_HUGETLB|MAP_HUGE_2MB flags on ::mmap here:
            //    Za. requires previously-reserved memory in /proc/sys/vm/nr_hugepages
            //    Zb. reserved pages permenently resident in RAM, never swapped
            //    Zc. memory cost incurred even if no application is using said pages

            std::size_t z = cfg.size_;

            z = padding::with_padding(z, config_.hugepage_z_); // 4.

            // 5.
            byte * base = reinterpret_cast<byte *>(
                ::mmap(nullptr,
                       z + config_.hugepage_z_,
                       PROT_NONE,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1, 0));

#ifdef NOT_YET
            log && log("acquired memory [lo,hi) using mmap",
                       xtag("lo", base),
                       xtag("z", z),
                       xtag("hi", reinterpret_cast<byte *>(base) + z));
#endif

            if (base == MAP_FAILED) {
                assert(false);
#ifdef NOPE
                throw std::runtime_error(tostr("ArenaAlloc: uncommitted allocation failed",
                                               xtag("size", z)));
#endif
            }

            byte * aligned_base = reinterpret_cast<byte *>
                (padding::with_padding(reinterpret_cast<size_t>(base),
                                       config_.hugepage_z_));

            assert(reinterpret_cast<size_t>(aligned_base) % config_.hugepage_z_ == 0);
            assert(aligned_base >= base);
            assert(aligned_base < base + config_.hugepage_z_);

            if (base < aligned_base) {
                size_t prefix = aligned_base - base;

                ::munmap(base, prefix); // 6.
            }

            byte * aligned_hi = aligned_base + z;
            byte * hi = base + z + config_.hugepage_z_;

            if (aligned_hi < hi) {
                size_t suffix = hi - aligned_hi;

                ::munmap(aligned_hi, suffix); // 7.
            }

#ifdef __linux__
            /** opt-in to huge pages, provided they're available.
             *  otherwise fallback gracefully
             **/
            ::madvise(aligned_base, z, MADV_HUGEPAGE); // 8.
#endif
            // TODO: for OSX -> need something else here.
            //       MAP_ALIGNED_SUPER with mmap() and/or
            //       use mach_vm_allocate()
            //

            this->lo_          = aligned_base;
            this->committed_z_ = 0;
            //this->checkpoint_  = lo_;
            this->free_        = lo_;
            this->limit_       = lo_;
            this->hi_          = lo_ + z;

            if (!lo_) {
                assert(false);
#ifdef NOPE
                throw std::runtime_error(tostr("ArenaAlloc: allocation failed",
                                               xtag("size", z)));
#endif
            }

#ifdef NOPE
            log && log(xtag("lo", (void*)lo_),
                       xtag("page_z", page_z_),
                       xtag("hugepage_z", hugepage_z_));
#endif
        }

        DArena::~DArena()
        {
            if (lo_) {
                //log && log("unmap [lo,hi)",
                //   xtag("lo", lo_),
                //   xtag("z", hi_ - lo_),
                //   xtag("hi", hi_));

                ::munmap(lo_, hi_ - lo_);
            }

            // hygiene
            lo_ = nullptr;
            committed_z_ = 0;
            // checkpoint_ = nullptr;
            free_ = nullptr;
            limit_ = nullptr;
            hi_ = nullptr;
        }
    }
} /*namespace xo*/

/* end DArena.cpp */
