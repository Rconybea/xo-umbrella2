/** @file IAllocator_DArena.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "IAllocator_DArena.hpp"
#include "padding.hpp"
#include "xo/indentlog/scope.hpp"
#include <cassert>
#include <cstddef>
#include <cstring>
#include <sys/mman.h>

namespace xo {
    using std::size_t;
    using std::byte;

    namespace mm {

        std::string_view
        IAllocator_DArena::name(const DArena & s) noexcept {
            return s.config_.name_;
        }

        size_t
        IAllocator_DArena::reserved(const DArena & s) noexcept {
            return s.hi_ - s.lo_;
        }

        size_t
        IAllocator_DArena::size(const DArena & s) noexcept {
            return s.limit_ - s.lo_;
        }

        size_t
        IAllocator_DArena::committed(const DArena & s) noexcept {
            return s.committed_z_;
        }

        size_t
        IAllocator_DArena::available(const DArena & s) noexcept {
            return s.limit_ - s.free_;
        }

        size_t
        IAllocator_DArena::allocated(const DArena & s) noexcept {
            return s.free_ - s.lo_;
        }

        bool
        IAllocator_DArena::contains(const DArena & s,
                                    const void * p) noexcept
        {
            return (s.lo_ <= p) && (p < s.hi_);
        }

        AllocatorError
        IAllocator_DArena::last_error(const DArena & s) noexcept {
            return s.last_error_;
        }

        bool
        IAllocator_DArena::expand(DArena & s, size_t target_z) noexcept
        {
            scope log(XO_DEBUG(s.config_.debug_flag_),
                      xtag("target_z", target_z),
                      xtag("committed_z", s.committed_z_));

            if (target_z <= s.committed_z_) [[likely]] {
                log && log("trivial success, offset within committed range",
                           xtag("target_z", target_z),
                           xtag("committed_z", s.committed_z_));
                return true;
            }

            if (s.lo_ + target_z > s.hi_) [[unlikely]] {
                ++(s.error_count_);
                s.last_error_ = AllocatorError(error::reserve_exhausted,
                                               s.error_count_,
                                               target_z, s.committed_z_, reserved(s));
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

            std::size_t aligned_target_z = padding::with_padding(target_z, s.config_.hugepage_z_);
            std::byte * commit_start = s.limit_; // = s.lo_ + s.committed_z_;
            std::size_t add_commit_z = aligned_target_z - s.committed_z_;

            assert(s.limit_ == s.lo_ + s.committed_z_);

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
                ++(s.error_count_);
                s.last_error_ = AllocatorError(error::commit_failed,
                                               s.error_count_,
                                               add_commit_z, s.committed_z_, reserved(s));
#ifdef OBSOLETE
                throw std::runtime_error(tostr("ArenaAlloc::expand: commit failure",
                                               xtag("committed_z", s.committed_z_),
                                               xtag("add_commit_z", add_commit_z)));
#endif
                return false;
            }

            s.committed_z_ = aligned_target_z;
            s.limit_ = s.lo_ + s.committed_z_;

            if (commit_start == s.lo_) [[unlikely]]
            {
                /* first expand() for this allocator - start with guard_z_ bytes */

                ::memset(s.free_,
                         s.config_.guard_byte_,
                         s.config_.guard_z_);

                s.free_ += s.config_.guard_z_;
            }

            assert(s.committed_z_ % s.config_.hugepage_z_ == 0);
            assert(reinterpret_cast<size_t>(s.limit_) % s.config_.hugepage_z_ == 0);

            return true;
        } /*expand*/

        std::byte *
        IAllocator_DArena::alloc(DArena & s,
                                 std::size_t req_z)
        {
            /* - primary allocation path:
             *   exactly 1 header per alloc() call.
             * - store_header_flag follows configuration
             */

            return _alloc(s, req_z,
                          alloc_mode::standard);
        }

        std::byte *
        IAllocator_DArena::super_alloc(DArena & s,
                                       std::size_t req_z)
        {
            /* - (uncommon) pattern for parent alloc immediately followed by
             *   zero-or-more susidiary allocs, all sharing a single header.
             * - collapses into alloc() behavior when
             *   ArenaConfig.store_header_flag_ disabled
             */

            return _alloc(s, req_z,
                          alloc_mode::super);
        }

        std::byte *
        IAllocator_DArena::sub_alloc(DArena & s,
                                     std::size_t req_z,
                                     bool complete_flag)
        {
            /* - (uncommon) pattern for subsidiary allocs:
             *   that piggyback onto preceding super_alloc()
             * - collapses into alloc() behavior when
             *   ArenaConfig.store_header_flag_ disabled
             */

            return _alloc(s, req_z,
                          (complete_flag
                           ? alloc_mode::sub_complete
                           : alloc_mode::sub_incomplete));

#ifdef OBSOLETE
            if ((req_z == 0) && complete_flag) [[unlikely]] {
                /** use zero req_z with complete_flag to clear s.last_header_ **/

                if (s.config_.store_header_flag_) {
                    if (!s.last_header_) [[unlikely]] {
                        ++(s.error_count_);
                        s.last_error_ = AllocatorError(error::orphan_sub_alloc,
                                                       s.error_count_,
                                                       0 /*add_commit_z*/, s.committed_z_, reserved(s));
                    } else {
                        s.last_header_ = nullptr;
                    }
                }

                return nullptr;
            }

            byte * free0 = s.free_;
            byte * mem = _alloc(s, req_z,
                                complete_flag ? alloc_mode::sub_complete : alloc_mode::sub,
                                false /*!store_header_flag*/,
                                false /*!remember_header_flag*/);

            if (!mem) [[unlikely]] {
                /* error already captured */
                return nullptr;
            }

            byte * free1 = s.free_;
            /* used: accounting for padding applied to req_z */
            size_t z0 = (free1 - free0);

            assert(z0 > 0);

            if (s.config_.store_header_flag_) {
                if (!s.last_header_) [[unlikely]] {
                    ++(s.error_count_);
                    s.last_error_ = AllocatorError(error::orphan_sub_alloc,
                                                   s.error_count_,
                                                   0 /*add_commit_z*/, s.committed_z_, reserved(s));
                    return nullptr;
                }

                /* s.last_header_ holds aggregate size of preceding super_alloc
                 * (+ any sub-alloc's).
                 *
                 * Accumulate allocation size
                 */
                uint64_t header = *s.last_header_;

                if ((header & s.config_.header_size_mask_ & z0) != z0) [[unlikely]] {
                    /* cumulative alloc size doesn't fit in configured header_size_mask bits */
                    ++(s.error_count_);
                    s.last_error_ = AllocatorError(error::header_size_mask,
                                                   s.error_count_,
                                                   0 /*add_commit_z*/, s.committed_z_, reserved(s));
                    return nullptr;
                }

                *s.last_header_ = ((header & ~s.config_.header_size_mask_) | z0);

                if (complete_flag) {
                    s.last_header_ = nullptr;
                }
            }

            return mem;
#endif
        }

        byte *
        IAllocator_DArena::_alloc(DArena & s,
                                  std::size_t req_z,
                                  alloc_mode mode)
        {
            scope log(XO_DEBUG(s.config_.debug_flag_));

            /*
             *                                                     sub_complete
             *                                            sub_incomplete      |
             *                                    standard  super      |      |
             *                                           v      v      v      v
             */
            std::array<bool, 4>  store_header_v = {{  true,  true, false, false }};
            std::array<bool, 4> retain_header_v = {{ false,  true, false, false }};
            std::array<bool, 4>   store_guard_v = {{  true, false, false,  true }};

            /* -> write header at s.free_ */
            bool store_header_flag = false;
            /* -> stash s.last_header_*/
            bool retain_header_flag = false;
            /* -> write guard bytes */
            bool store_guard = false;

            if (s.config_.store_header_flag_) {
                store_header_flag  = store_header_v[(int)mode];
                retain_header_flag = retain_header_v[(int)mode];
                store_guard        = store_guard_v[(int)mode];
            }

            assert(padding::is_aligned((size_t)s.free_));

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
                if ((s.config_.header_size_mask_ & z0) == z0) [[likely]] {
                    hz = sizeof(header);
                } else {
                    /* req_z doesn't fit in configured header_size_mask bits */
                    ++(s.error_count_);
                    s.last_error_ = AllocatorError(error::header_size_mask,
                                                   s.error_count_,
                                                   0 /*add_commit_z*/,
                                                   s.committed_z_,
                                                   reserved(s));
                    return nullptr;
                }
            }

            size_t z1 = hz + z0;

            assert(padding::is_aligned(z1));

            if (!expand(s, allocated(s) + z1)) [[unlikely]] {
                /* (error state already captured) */
                return nullptr;
            }

            if (store_header_flag) {
                /* capturing header */
                *(uint64_t *)s.free_ = header;

                if (retain_header_flag) {
                    /* and rembering for subsequent
                     *   sub_alloc()
                     */
                    s.last_header_ = (uint64_t *)s.free_;
                }
            }

            byte * mem = s.free_ + hz;

            s.free_ += z1;

            if (store_guard) {
                /* write guard bytes for overrun detection */
                ::memset(s.free_,
                         s.config_.guard_byte_,
                         s.config_.guard_z_);

                s.free_ += s.config_.guard_z_;
            }

            log && log(xtag("self", s.config_.name_),
                       xtag("hz", hz),
                       xtag("z0", req_z),
                       xtag("+pad", dz),
                       xtag("z1", z1),
                       xtag("size", size(s)),
                       xtag("avail", available(s)));

            return mem;
        }

        void
        IAllocator_DArena::clear(DArena & s)
        {
            s.free_ = s.lo_;
            //s.checkpoint_ = s.lo_;
        }

        void
        IAllocator_DArena::destruct_data(DArena & s)
        {
            s.~DArena();
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_DArena.cpp */
