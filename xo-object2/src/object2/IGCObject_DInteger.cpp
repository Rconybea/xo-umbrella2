/** @file IGCObject_DInteger.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "IGCObject_DInteger.hpp"
#include "xo/alloc2/alloc/AAllocator.hpp"
#include "xo/facet/obj.hpp"
#include <cstddef>

namespace xo {
    using xo::mm::AAllocator;
    using xo::facet::obj;
    using std::size_t;

    namespace scm {
        size_t
        IGCObject_DInteger::shallow_size(const DInteger &) noexcept
        {
            return sizeof(DInteger);
        }

        DInteger *
        IGCObject_DInteger::shallow_copy(const DInteger & src,
                                       obj<AAllocator> mm) noexcept
        {
            DInteger * copy = (DInteger *)mm.alloc(sizeof(DInteger));

            if (copy)
                *copy = src;

            return copy;
        }

        size_t
        IGCObject_DInteger::forward_children(DInteger &) noexcept
        {
            return sizeof(DInteger);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DInteger.cpp */
