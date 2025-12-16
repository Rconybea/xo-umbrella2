/** @file DArena.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "alloc/AAllocator.hpp"
#include "arena/DArena.hpp"
#include "xo/alloc2/padding.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <cassert>
#include <sys/mman.h> // for ::munmap()
#include <unistd.h> // for ::getpagesize()
#include <string.h> // for ::memset()

namespace xo {
    using std::byte;
    using std::size_t;

    namespace mm {

        /** Map a contiguous uncommitted memory range comprising
         *  a whole multiple of @p hugepage_z, with at least
         *  @p req_z bytes.
         *
         *  Memory will also be aligned on @p hugepage_z boundary
         *  (2MB in practice)
         *
         *  - @p req_z is rounded up to a multiple of @p hugepage_z
         *  - Resulting uncommitted address range not backed by
         *    physical memory.
         *  - since hugpage-aligned, can find base of mapped range
         *    by masking off the bottom log2(align_z) bits.
         *    May rely on this for GC metadata
         *  - opt-in to transparent huge pages (THP).
         *    Reduces page-fault time by a lot, in return for
         *    lower VM granularity
         * - rejecting inferior MAP_HUGETLB|MAP_HUGE_2MB flags on ::mmap here:
         *    - requires previously-reserved memory in /proc/sys/vm/nr_hugepages
         *    - reserved pages permenently resident in RAM, never swapped
         *    - memory cost incurred even if no application is using said pages
         *
         * TODO: for OSX -> need something else here.
         *       MAP_ALIGNED_SUPER with mmap() and/or
         *       use mach_vm_allocate()
         *
         *  @return pair giving mapped address range [lo, hi)
         **/
        auto
        DArena::map_aligned_range(size_t req_z, size_t hugepage_z) -> range_type
        {
            // 1. round up to multiple of hugepage_z
            size_t target_z = padding::with_padding(req_z, hugepage_z); // 4.

            // 2. mmap() will give us page-aligned memory,
            //    but not hugepage-aligned.
            //
            //    Over-request by hugepage_z to ensure
            //    hugepage-aligned subrange of size target_z
            //
            byte * base = (byte *)(::mmap(nullptr,
                                          target_z + hugepage_z,
                                          PROT_NONE,
                                          MAP_PRIVATE | MAP_ANONYMOUS,
                                          -1, 0));

            // on mmap success: upper limit of mapped address range
            byte * hi = base + (target_z + hugepage_z);
            // lowest hugepage-aligned address in [base, hi)
            byte * aligned_base = (byte *)(padding::with_padding((size_t)base, hugepage_z));
            // end of hugeppage-aligned range starting at aligned_base
            byte * aligned_hi = aligned_base + target_z;

#ifdef NOT_YET
            log && log("acquired memory [lo,hi) using mmap",
                       xtag("lo", base),
                       xtag("req_z", req_z),
                       xtag("target_z", target_z),
                       xtag("hi", (byte *)(base) + z));
#endif


            // 3. assess mmap success
            {
                if (base == MAP_FAILED) {
                    throw std::runtime_error(tostr("ArenaAlloc: uncommitted allocation failed",
                                                   xtag("size", req_z)));
                }

                assert((size_t)aligned_base % hugepage_z == 0);
                assert(aligned_base >= base);
                assert(aligned_base < base + hugepage_z);
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
             **/
            ::madvise(aligned_base, target_z, MADV_HUGEPAGE); // 8.
#endif

            return std::make_pair(aligned_base, aligned_hi);
        }

        DArena
        DArena::map(const ArenaConfig & cfg)
        {
            //scope log(XO_DEBUG(debug_flag), xtag("name", name));

            auto [lo, hi]     = map_aligned_range(cfg.size_, cfg.hugepage_z_);

            if (!lo) {
                // control here implies mmap() failed silently

                throw std::runtime_error(tostr("ArenaAlloc: allocation failed",
                                               xtag("size", cfg.size_)));
            }

            size_t page_z = getpagesize();


#ifdef NOPE
            log && log(xtag("lo", (void*)lo_),
                       xtag("page_z", page_z_),
                       xtag("hugepage_z", hugepage_z_));
#endif

            return DArena(cfg, page_z, lo, hi);
        } /*map*/

        DArena::DArena(const ArenaConfig & cfg,
                       size_type page_z,
                       byte * lo,
                       byte * hi) : config_{cfg},
                                    page_z_{page_z},
                                    lo_{lo},
                                    committed_z_{0},
                                    free_{lo},
                                    limit_{lo},
                                    hi_{hi},
                                    error_count_{0},
                                    last_error_{}
        {
            //retval.checkpoint_  = lo_;

            /** make sure guard size is aligned **/
            config_.guard_z_ = padding::with_padding(config_.guard_z_);
        }

        DArena::DArena(DArena && other) {
            config_            = other.config_;
            page_z_            = other.page_z_;
            lo_                = other.lo_;
            committed_z_       = other.committed_z_;
            free_              = other.free_;
            limit_             = other.limit_;
            hi_                = other.hi_;
            error_count_       = other.error_count_;
            last_error_        = other.last_error_;

            other.config_      = ArenaConfig();
            other.lo_          = nullptr;
            other.committed_z_ = 0;
            other.free_        = nullptr;
            other.limit_       = nullptr;
            other.hi_          = nullptr;
            other.error_count_ = 0;
            other.last_error_  = AllocError();
        }

        DArena &
        DArena::operator=(DArena && other)
        {
            config_            = other.config_;
            page_z_            = other.page_z_;
            lo_                = other.lo_;
            committed_z_       = other.committed_z_;
            free_              = other.free_;
            limit_             = other.limit_;
            hi_                = other.hi_;
            error_count_       = other.error_count_;
            last_error_        = other.last_error_;

            other.config_      = ArenaConfig();
            other.lo_          = nullptr;
            other.committed_z_ = 0;
            other.free_        = nullptr;
            other.limit_       = nullptr;
            other.hi_          = nullptr;
            other.error_count_ = 0;
            other.last_error_  = AllocError();

            return *this;
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
            lo_            = nullptr;
            committed_z_   = 0;
            // checkpoint_ = nullptr;
            free_          = nullptr;
            limit_         = nullptr;
            hi_            = nullptr;
            error_count_   = 0;
            last_error_    = AllocError();
        }

        DArena::header_type *
        DArena::obj2hdr(void * obj) noexcept
        {
            assert(config_.store_header_flag_);

            return (header_type *)((byte *)obj - sizeof(header_type));
        }

        AllocInfo
        DArena::alloc_info(value_type mem) noexcept
        {
            if (!config_.store_header_flag_) [[unlikely]] {
                ++(error_count_);
                last_error_ = AllocError(error::alloc_info_disabled,
                                         error_count_,
                                         0 /*add_commit_z*/,
                                         committed_z_,
                                         this->reserved());

                return AllocInfo::error_not_configured(&config_.header_);
            }

            byte * header_mem = mem - sizeof(AllocHeader);

            if (!this->contains(header_mem)) {
                ++(error_count_);
                last_error_ = AllocError(error::alloc_info_address,
                                         error_count_,
                                         0 /*add_commit_z*/,
                                         committed_z_,
                                         this->reserved());
            }

            return AllocInfo(&config_.header_, (AllocHeader *)header_mem);
        }

        void
        DArena::clear() noexcept
        {
            this->free_ = lo_;
        }
    }
} /*namespace xo*/

/* end DArena.cpp */
