/** @file IAllocator_DArena.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "IAllocator_DArena.hpp"
#include <cassert>
#include <cstddef>

namespace xo {
    namespace mm {

        const std::string &
        IAllocator_DArena::name(const DArena & s) {
            return s.config_.name_;
        }

        std::size_t
        IAllocator_DArena::reserved(const DArena & s) {
            return s.hi_ - s.lo_;
        }

        std::size_t
        IAllocator_DArena::size(const DArena & s) {
            return s.limit_ - s.lo_;
        }

        std::size_t
        IAllocator_DArena::committed(const DArena & s) {
            return s.committed_z_;
        }

        bool
        IAllocator_DArena::contains(const DArena & s,
                                          const void * p)
        {
            return (s.lo_ <= p) && (p < s.hi_);
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
