/** @file RAllocator_aux.hpp
 *
 *  Out-of-line definitions for RAllocator template methods
 *  that depend on RGCObject (avoiding #include cycle in RAllocator.hpp)
 *
 *  Would aspire to include via user_hpp_includes in Allocator.json5,
 *  if/when that exists
 *
 *  @author Roland Conybeare
 **/

#pragma once

#include "RAllocator.hpp"
#include "GCObject.hpp"

namespace xo {
    namespace mm {

        /** NOTE:
         *  this definition is incorporated into [Allocator.hpp],
         *  while consciously omitted from [Allocator_basic.hpp].
         *
         *  Some .hpp files in {xo-alloc2/, xo-gc/} can
         *  only include [Allocator_basic.hpp].
         *
         *  Translation units that want to invoke
         *  barrier_assign() must #include Allocator.hpp
         **/
        template <typename Object>
        void
        RAllocator<Object>::barrier_assign(void * parent,
                                           obj<AGCObject> * p_lhs,
                                           obj<AGCObject> rhs) noexcept
        {
            this->barrier_assign_aux(parent,
                                     p_lhs->iface(), p_lhs->opaque_data_addr(),
                                     rhs.iface(), rhs.opaque_data());
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end RAllocator_aux.hpp */
