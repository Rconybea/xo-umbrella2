/** @file DList.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "xo/alloc2/gc/RGCObject.hpp"
#include "xo/alloc2/gc/IGCObject_Any.hpp"
#include "xo/facet/obj.hpp"

namespace xo {
    namespace scm {

        struct DList {
            using AGCObject = xo::mm::AGCObject;

            DList(xo::obj<AGCObject> h,
                  xo::obj<AGCObject> r) : head_{h}, rest_{r} {}

            obj<AGCObject> head_;
            obj<AGCObject> rest_;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DList.hpp */
