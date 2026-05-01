/** @file RAllocator_aux.hpp
 *
 *  Out-of-line definitions for RAllocator template methods
 *  that depend on RGCObject (avoiding #include cycle in RAllocator.hpp)
 *
 *  Would aspire to Include via user_hpp_includes in Allocator.json5,
 *  if/when that exists
 *
 *  @author Roland Conybeare
 **/

#pragma once

#include "RAllocator.hpp"
#include "GCObject.hpp"

namespace xo {
    namespace mm {

        template <typename Object>
        void
        RAllocator<Object>::barrier_assign(void * parent,
                                           obj<AGCObject> * p_lhs,
                                           obj<AGCObject> rhs) noexcept
        {
            (void)parent;
            (void)p_lhs;
            (void)rhs;
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end RAllocator_aux.hpp */
