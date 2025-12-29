/** @file IGCObject_DList.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "IGCObject_DList.hpp"

namespace xo {
    using xo::mm::AGCObject;
    using xo::mm::AAllocator;
    using xo::facet::with_facet;
    using xo::facet::obj;
    using std::size_t;

    namespace scm {
        size_t
        IGCObject_DList::shallow_size(const DList &) noexcept
        {
            return sizeof(DList);
        }

        DList *
        IGCObject_DList::shallow_copy(const DList & src,
                                      obj<AAllocator> mm) noexcept
        {
            DList * copy = (DList *)mm.alloc(sizeof(DList));

            if (copy)
                *copy = src;

            return copy;
        }

        size_t
        IGCObject_DList::forward_children(DList & src,
                                          obj<ACollector> gc) noexcept
        {
            gc.forward_inplace(src.head_.iface(), (void **)&(src.head_.data_));

            //auto rest = with_facet<AGCObject>::mkobj(src.rest_);
            xo::facet::FacetImplementation<xo::mm::AGCObject, DList>::ImplType iface;
            gc.forward_inplace(&iface, (void **)(&src.rest_));

            return shallow_size(src);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DList.cpp */
