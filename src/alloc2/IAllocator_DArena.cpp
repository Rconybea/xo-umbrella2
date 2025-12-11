/** @file IAllocator_DArena.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "IAllocator_DArena.hpp"
#include <cassert>

namespace xo {
    namespace mm {

        std::size_t
        IAllocator_Impl<DArena>::size(const DArena & s) {
            return s.limit_ - s.lo_;
        }

        std::size_t
        IAllocator_Impl<DArena>::committed(const DArena & s) {
            return s.committed_z_;
        }

        bool
        IAllocator_Impl<DArena>::contains(const DArena & s,
                                          const void * p)
        {
            return (s.lo_ <= p) && (p < s.hi_);
        }

        std::byte *
        IAllocator_Impl<DArena>::alloc(const DArena & s,
                                       std::size_t z)
        {
            (void)s;
            (void)z;

            // scope log(XO_DEBUG(config_.debug_flag_));

            assert(false);
        }

        void
        IAllocator_Impl<DArena>::clear(DArena & s)
        {
            s.free_ = s.lo_;
            //s.checkpoint_ = s.lo_;
        }

         void
         IAllocator_Impl<DArena>::destruct_data(DArena & s)
         {
             s.~DArena();
         }
    } /*namespace mm*/
} /*namespace xo*/

/* end IAllocator_DArena.cpp */
