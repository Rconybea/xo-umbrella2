/** @file DArena.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "alloc/AAllocator.hpp"
#include "arena/DArena.hpp"
#include "arena/DArenaIterator.hpp"
#include "xo/alloc2/padding.hpp"
#include "xo/indentlog/scope.hpp"
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
            config_.header_.guard_z_
                = padding::with_padding(config_.header_.guard_z_);
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
        DArena::alloc_info(value_type mem) const noexcept
        {
            if (!config_.store_header_flag_) [[unlikely]] {
                this->capture_error(error::alloc_info_disabled);

                return AllocInfo::error_not_configured(&config_.header_);
            }

            byte * header_mem = mem - sizeof(AllocHeader);

            if (!this->contains(header_mem)) {
                this->capture_error(error::alloc_info_address);
            }

            AllocHeader * header = (AllocHeader *)header_mem;

            const byte * guard_lo
                = header_mem - config_.header_.guard_z_;
            const byte * guard_hi
                = mem + config_.header_.size(*header);

            return AllocInfo(&config_.header_,
                             guard_lo,
                             (AllocHeader *)header_mem,
                             guard_hi);
        }

        DArenaIterator
        DArena::begin() const noexcept
        {
            return DArenaIterator::begin(this);
        }

        DArenaIterator
        DArena::end() const noexcept
        {
            return DArenaIterator::end(this);
        }

        AllocHeader *
        DArena::begin_header() const noexcept
        {
            if (config_.store_header_flag_ == false) {
                this->capture_error(error::alloc_iterator_not_supported);

                return nullptr;
            }

            return (AllocHeader *)(lo_ + config_.header_.guard_z_);
        }

        AllocHeader *
        DArena::end_header() const noexcept
        {
            if (config_.store_header_flag_ == false) {
                this->capture_error(error::alloc_iterator_not_supported);

                return nullptr;
            }

            return (AllocHeader *)free_;
        }

        std::byte *
        DArena::alloc(std::size_t req_z)
        {
            /* - primary allocation path:
             *   exactly 1 header per alloc() call.
             * - store_header_flag follows configuration
             */

            return _alloc(req_z, alloc_mode::standard);
        }

        std::byte *
        DArena::super_alloc(std::size_t req_z)
        {
            /* - (uncommon) pattern for parent alloc immediately followed by
             *   zero-or-more susidiary allocs, all sharing a single header.
             * - collapses into alloc() behavior when
             *   ArenaConfig.store_header_flag_ disabled
             */

            return _alloc(req_z,
                          alloc_mode::super);
        }

        std::byte *
        DArena::sub_alloc(std::size_t req_z,
                          bool complete_flag)
        {
            /* - (uncommon) pattern for subsidiary allocs:
             *   that piggyback onto preceding super_alloc()
             * - collapses into alloc() behavior when
             *   ArenaConfig.store_header_flag_ disabled
             */

            return _alloc(req_z,
                          (complete_flag
                           ? alloc_mode::sub_complete
                           : alloc_mode::sub_incomplete));

        }

        void
        DArena::capture_error(error err,
                              size_type target_z) const
        {
            DArena * self = const_cast<DArena *>(this);

            ++(self->error_count_);
            self->last_error_ = AllocError(err,
                                           error_count_,
                                           target_z,
                                           committed_z_,
                                           reserved());
        }

        byte *
        DArena::_alloc(std::size_t req_z, alloc_mode mode)
        {
            scope log(XO_DEBUG(config_.debug_flag_));

            /*
             *                                                     sub_complete
             *                                            sub_incomplete      |
             *                                    standard  super      |      |
             *                                           v      v      v      v
             */
            std::array<bool, 4>  store_header_v = {{  true,  true, false, false }};
            std::array<bool, 4> retain_header_v = {{ false,  true, false, false }};
            std::array<bool, 4>   store_guard_v = {{  true, false, false,  true }};

            /* -> write header at free_ */
            bool store_header_flag = false;
            /* -> stash last_header_*/
            bool retain_header_flag = false;
            /* -> write guard bytes */
            bool store_guard = false;

            if (config_.store_header_flag_) {
                store_header_flag  = store_header_v[(int)mode];
                retain_header_flag = retain_header_v[(int)mode];
                store_guard        = store_guard_v[(int)mode];
            }

            assert(padding::is_aligned((size_t)free_));

            /*
             *                    free_(pre)
             *                    v
             *
             *                    <-------------z1--------------->
             *           < guard ><  hz  ><     req_z     >< dz  >< guard >
             *
             * used <==  +++++++++0000zzzz@@@@@@@@@@@@@@@@@ppppppp+++++++++ ==> avail
             *
             *                    ^       ^                                ^
             *                    header  mem                              |
             *                    ^                                        |
             *                    last_header_                   free_(post)
             *
             *            [+] guard after each allocation, for simple sanitize checks
             *            [0] unused header bits (avail to application)
             *            [z] record allocation size
             *            [@] new allocated memory
             *            [p] padding (to uintptr_t alignment)
             */

            /* non-zero if header feature enabled */
            size_t hz = 0;
            /* dz: pad req_z to alignment size (multiple of 8 bytes, probably) */
            size_t dz = padding::alloc_padding(req_z);
            size_t z0 = req_z + dz;
            /* if non-zero:
             *  will store padded alloc size at the beginning of each allocation
             * reminder:
             *  important to store padded size for correct arena iteration
             */
            uint64_t header = req_z + dz;

            if (store_header_flag)
            {
                if (config_.header_.is_size_enabled()) [[likely]] {
                    hz = sizeof(header);
                } else {
                    /* req_z doesn't fit in configured header_size_mask bits */
                    capture_error(error::header_size_mask);
                    return nullptr;
                }
            }

            size_t z1 = hz + z0;

            assert(padding::is_aligned(z1));

            if (!this->expand(this->allocated() + z1)) [[unlikely]] {
                /* (error state already captured) */
                return nullptr;
            }

            if (store_header_flag) {
                /* capturing header */
                *(uint64_t *)free_ = header;

                if (retain_header_flag) {
                    /* and rembering for subsequent
                     *   sub_alloc()
                     */
                    last_header_ = (AllocHeader *)free_;
                }
            }

            byte * mem = free_ + hz;

            this->free_ += z1;

            if (store_guard) {
                /* write guard bytes for overrun detection */
                ::memset(free_,
                         config_.header_.guard_byte_,
                         config_.header_.guard_z_);

                this->free_ += config_.header_.guard_z_;
            }

            log && log(xtag("self", config_.name_),
                       xtag("hz", hz),
                       xtag("z0", req_z),
                       xtag("+pad", dz),
                       xtag("z1", z1),
                       xtag("size", this->committed()),
                       xtag("avail", this->available()));
            log && log(xtag("mem", mem),
                       xtag("free", free_));

            return mem;
        }

        bool
        DArena::expand(size_t target_z) noexcept
        {
            scope log(XO_DEBUG(config_.debug_flag_),
                      xtag("target_z", target_z),
                      xtag("committed_z", committed_z_));

            if (target_z <= committed_z_) [[likely]] {
                log && log("trivial success, offset within committed range",
                           xtag("target_z", target_z),
                           xtag("committed_z", committed_z_));
                return true;
            }

            if (lo_ + target_z > hi_) [[unlikely]] {
                this->capture_error(error::reserve_exhausted, target_z);
                return false;
            }

            /*
             * pre:
             *
             *   _______________...................................
             *   ^              ^                                  ^
             *   lo         limit                                 hi
             *
             *   < committed_z >
             *   <----------target_z----------->
             *                                 >     <- z: 0 <= z < hugepage_z
             *   <---------aligned_target_z--------->
             *                  <--- add_commit_z -->
             *
             * post:
             *   ____________________________________..............
             *   ^                                   ^             ^
             *   lo                              limit            hi
             *
             */

            std::size_t aligned_target_z = padding::with_padding(target_z, config_.hugepage_z_);
            std::byte * commit_start = limit_; // = lo_ + committed_z_;
            std::size_t add_commit_z = aligned_target_z - committed_z_;

            assert(limit_ == lo_ + committed_z_);

            //            log && log(xtag("aligned_offset_z", aligned_offset_z),
            //                       xtag("add_commit_z", add_commit_z));
            //            log && log("expand committed range",
            //                       xtag("commit_start", commit_start),
            //                       xtag("add_commit_z", add_commit_z),
            //                       xtag("commit_end", commit_start + add_commit_z));

            if (::mprotect(commit_start,
                           add_commit_z,
                           PROT_READ | PROT_WRITE) != 0) [[unlikely]]
                {
                    capture_error(error::commit_failed, add_commit_z);
                    return false;
                }

            committed_z_ = aligned_target_z;
            limit_ = lo_ + committed_z_;

            if (commit_start == lo_) [[unlikely]] {
                /* first expand() for this allocator - start with guard_z_ bytes */

                ::memset(free_,
                         config_.header_.guard_byte_,
                         config_.header_.guard_z_);

                free_ += config_.header_.guard_z_;
            }

            assert(committed_z_ % config_.hugepage_z_ == 0);
            assert(reinterpret_cast<size_t>(limit_) % config_.hugepage_z_ == 0);

            return true;
        } /*expand*/

        void
        DArena::clear() noexcept
        {
            this->free_ = lo_;
        }
    }
} /*namespace xo*/

/* end DArena.cpp */
