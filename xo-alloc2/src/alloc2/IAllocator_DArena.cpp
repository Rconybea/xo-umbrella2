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

    namespace mm {

        const std::string &
        IAllocator_DArena::name(const DArena & s) {
            return s.config_.name_;
        }

        size_t
        IAllocator_DArena::reserved(const DArena & s) {
            return s.hi_ - s.lo_;
        }

        size_t
        IAllocator_DArena::size(const DArena & s) {
            return s.limit_ - s.lo_;
        }

        size_t
        IAllocator_DArena::committed(const DArena & s) {
            return s.committed_z_;
        }

        size_t
        IAllocator_DArena::available(const DArena & s) {
            return s.limit_ - s.free_;
        }

        size_t
        IAllocator_DArena::allocated(const DArena & s) {
            return s.free_ - s.lo_;
        }

        bool
        IAllocator_DArena::contains(const DArena & s,
                                          const void * p)
        {
            return (s.lo_ <= p) && (p < s.hi_);
        }

        bool
        IAllocator_DArena::expand(DArena & s, size_t target_z)
        {
            scope log(XO_DEBUG(s.config_.debug_flag_),
                      xtag("target_z", target_z),
                      xtag("committed_z", s.committed_z_));

            if (target_z <= s.committed_z_) {
                log && log("trivial success, offset within committed range",
                           xtag("target_z", target_z),
                           xtag("committed_z", s.committed_z_));
                return true;
            }

            if (s.lo_ + target_z > s.hi_) {
                throw std::runtime_error(tostr("ArenaAlloc::expand: requested size exceeds reserved size",
                                               xtag("requested", target_z),
                                               xtag("reserved", reserved(s))));
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

            if (::mprotect(commit_start, add_commit_z, PROT_READ | PROT_WRITE) != 0) {
                assert(false);
//                throw std::runtime_error(tostr("ArenaAlloc::expand: commit failure",
//                                               xtag("committed_z", committed_z_),
//                                               xtag("add_commit_z", add_commit_z)));
                return false;
            }

            s.committed_z_ = aligned_target_z;
            s.limit_ = s.lo_ + s.committed_z_;

            assert(s.committed_z_ % s.config_.hugepage_z_ == 0);
            assert(reinterpret_cast<size_t>(s.limit_) % s.config_.hugepage_z_ == 0);

            return true;
        }

        std::byte *
        IAllocator_DArena::alloc(const DArena & s,
                                       std::size_t z)
        {
            (void)s;
            (void)z;

            // scope log(XO_DEBUG(config_.debug_flag_));

            assert(false);
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
