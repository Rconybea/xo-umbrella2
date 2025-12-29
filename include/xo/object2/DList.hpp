/** @file DList.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "xo/gc/GCObject.hpp"
//#include "xo/alloc2/gcobject/RGCObject.hpp"
#include "xo/facet/obj.hpp"

namespace xo {
    namespace scm {

        struct DList {
            using size_type = std::size_t;
            using AGCObject = xo::mm::AGCObject;

            DList(xo::obj<AGCObject> h,
                  DList * r) : head_{h}, rest_{r} {}

            /** DList length is at least 1 **/
            bool is_empty() const noexcept { return false; };
            /** DList models a finite sequence **/
            bool is_finite() const noexcept { return true; };
            /** return number of elements in this DList **/
            size_type size() const noexcept;
            /** return element at 0-based index @p ix **/
            obj<AGCObject> at(size_type ix) const;

            obj<AGCObject> head_;
            DList * rest_ = nullptr;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DList.hpp */
