/** @file IGCObject_DFloat.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "IGCObject_DFloat.hpp"
#include "xo/alloc2/alloc/AAllocator.hpp"
#include "xo/facet/obj.hpp"
#include <cstddef>

namespace xo {
    using xo::mm::AAllocator;
    using xo::facet::obj;
    using xo::facet::typeseq;
    using std::size_t;

    namespace scm {
        size_t
        IGCObject_DFloat::shallow_size(const DFloat &) noexcept
        {
            return sizeof(DFloat);
        }

        DFloat *
        IGCObject_DFloat::shallow_copy(const DFloat & src,
                                       obj<AAllocator> mm) noexcept
        {
            DFloat * copy = (DFloat *)mm.alloc(typeseq::id<DFloat>(),
                                               sizeof(DFloat));

            if (copy)
                *copy = src;

            return copy;
        }

        size_t
        IGCObject_DFloat::forward_children(DFloat & src,
                                           obj<ACollector>) noexcept
        {
            return shallow_size(src);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DFloat.cpp */
