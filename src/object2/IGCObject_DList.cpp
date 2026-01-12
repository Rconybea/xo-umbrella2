/** @file IGCObject_DList.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "IGCObject_DList.hpp"
#include "DList.hpp"
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::mm::AGCObject;
    using xo::mm::AAllocator;
    //using xo::facet::with_facet;
    using xo::facet::obj;
    using xo::facet::typeseq;
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
            DList * copy = (DList *)mm.alloc_copy((std::byte *)&src);

            if (copy)
                *copy = src;

            return copy;
        }

        size_t
        IGCObject_DList::forward_children(DList & src,
                                          obj<ACollector> gc) noexcept
        {
            scope log(XO_DEBUG(true));

            gc.forward_inplace(src.head_.iface(), (void **)&(src.head_.data_));

            auto iface = xo::facet::impl_for<AGCObject, DList>();
            gc.forward_inplace(&iface, (void **)(&src.rest_));

            return shallow_size(src);
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DList.cpp */
