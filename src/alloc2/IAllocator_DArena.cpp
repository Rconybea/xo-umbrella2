/** @file IAllocator_DArena.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "IAllocator_DArena.hpp"
#include "padding.hpp"
#include "xo/indentlog/scope.hpp"
#include <cassert>
#include <cstddef>
#include <sys/mman.h>

namespace xo {
    using std::size_t;
    using std::byte;

    namespace mm {

        const std::string &
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

#ifdef OBSOLETE
                throw std::runtime_error(tostr("ArenaAlloc::expand: requested size exceeds reserved size",
                                               xtag("requested", target_z),
                                               xtag("reserved", reserved(s))));
#endif
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

            if (::mprotect(commit_start, add_commit_z, PROT_READ | PROT_WRITE) != 0) [[unlikely]] {
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

            assert(s.committed_z_ % s.config_.hugepage_z_ == 0);
            assert(reinterpret_cast<size_t>(s.limit_) % s.config_.hugepage_z_ == 0);

            return true;
        }

        std::byte *
        IAllocator_DArena::alloc(DArena & s,
                                 std::size_t req_z)
        {
            scope log(XO_DEBUG(s.config_.debug_flag_));

            /* word size for alignment (8 bytes) */
            constexpr size_t c_bpw = sizeof(std::uintptr_t);

            std::uintptr_t free_u64 = reinterpret_cast<std::uintptr_t>(s.free_);

            assert(free_u64 % c_bpw == 0ul);

            /* dz: pad req_z to multiple c_bpw */
            size_t dz = padding::alloc_padding(req_z);
            size_t z1 = req_z + dz;

            assert(z1 % c_bpw == 0ul);

            if (expand(s, allocated(s) + z1)) [[likely]] {
                byte * mem = s.free_;

                s.free_ += z1;

                log && log(xtag("self", s.config_.name_),
                           xtag("z0", req_z),
                           xtag("+pad", dz),
                           xtag("z1", z1),
                           xtag("size", size(s)),
                           xtag("avail", available(s)));

                return mem;
            } else {
                /* error already captured */
                return nullptr;
            }
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
