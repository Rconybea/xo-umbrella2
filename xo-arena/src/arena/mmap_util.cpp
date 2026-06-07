/** @file mmap_util.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "mmap_util.hpp"
#include "padding.hpp"
#include <sys/mman.h> // for mmap

namespace xo {
    namespace mm {
        auto
        mmap_util::map_aligned_range(size_t req_z,
                                     size_t align_z,
                                     bool enable_hugepage_flag,
                                     bool debug_flag) -> span_type
        {
            scope log(XO_DEBUG(debug_flag),
                      xtag("req_z", req_z),
                      xtag("align_z", align_z));

            // 1. round up to multiple of align_z
            size_t target_z = padding::with_padding(req_z, align_z); // 4.

            // 2. mmap() will give us page-aligned memory,
            //    but not hugepage-aligned.
            //
            //    Over-request by align_z to ensure
            //    aligned subrange of size target_z
            //
            byte * base = (byte *)(::mmap(nullptr,
                                          target_z + align_z,
                                          PROT_NONE,
                                          MAP_PRIVATE | MAP_ANONYMOUS,
                                          -1, 0));

            // on mmap success: upper limit of mapped address range
            byte * hi = base + (target_z + align_z);
            // lowest hugepage-aligned address in [base, hi)
            byte * aligned_base = (byte *)(padding::with_padding((size_t)base, align_z));
            // end of hugeppage-aligned range starting at aligned_base
            byte * aligned_hi = aligned_base + target_z;

            log && log("acquired memory [lo,hi) using mmap",
                       xtag("lo", base),
                       xtag("aligned_lo", aligned_base),
                       xtag("req_z", req_z),
                       xtag("target_z", target_z),
                       xtag("aligned_hi", aligned_hi),
                       xtag("hi", hi));

            // 3. assess mmap success
            {
                if (base == MAP_FAILED) {
                    throw std::runtime_error(tostr("ArenaAlloc: uncommitted allocation failed",
                                                   xtag("size", req_z)));
                }

                assert((size_t)aligned_base % align_z == 0);
                assert(aligned_base >= base);
                assert(aligned_base < base + align_z);
            }

            // 4. release unaligned prefix
            if (base < aligned_base) {
                size_t ua_prefix = aligned_base - base;

                ::munmap(base, ua_prefix);
            }

            // 5. release unaligned suffix
            if (aligned_hi < hi) {
                size_t suffix = hi - aligned_hi;

                ::munmap(aligned_hi, suffix);
            }

            if (enable_hugepage_flag) {
#ifdef __linux__
                /** linux:
                 *    opt-in to transparent huge pages (THP)
                 *    provided OS configured to support them.
                 *    otherwise fallback gracefully.
                 *
                 *    Huge pages -> use fewer TLB entries + faster
                 *    shorter path through page table.
                 *
                 *    When we commit (i.e. obtain physical memory on page fault),
                 *    typically expect to pay ~1us per superpage.
                 *    Much better than ~500us to commit 512 4k VM pages.
                 *
                 *    But wasted if we don't use the memory.
                 *
                 *    Page table has a handful of levels
                 **/
                ::madvise(aligned_base, target_z, MADV_HUGEPAGE); // 8.
#endif
            }

            return span_type(aligned_base, aligned_hi);
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end mmap_util.cpp */
