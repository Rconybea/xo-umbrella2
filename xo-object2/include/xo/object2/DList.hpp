/** @file DList.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/gc/GCObject.hpp>
//#include "xo/alloc2/gcobject/RGCObject.hpp"
#include <xo/facet/obj.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>

namespace xo {
    namespace scm {

        struct DList {
            using size_type = std::size_t;
            using AGCObject = xo::mm::AGCObject;
            using AAllocator = xo::mm::AAllocator;
            using ppindentinfo = xo::print::ppindentinfo;

            DList(xo::obj<AGCObject> h,
                  DList * r) : head_{h}, rest_{r} {}

            /** sentinel for null list **/
            static DList * null();

            /** list with one element @p h1, allocated from @p mm **/
            static DList * list(obj<AAllocator> mm,
                                obj<AGCObject> h1);
            /** list with two elements @p h1, @p h2, allocated from @p mm **/
            static DList * list(obj<AAllocator> mm,
                                obj<AGCObject> h1,
                                obj<AGCObject> h2);

            /** DList length is at least 1 **/
            bool is_empty() const noexcept;
            /** DList models a finite sequence **/
            bool is_finite() const noexcept { return true; };
            /** return number of elements in this DList **/
            size_type size() const noexcept;
            /** return element at 0-based index @p ix **/
            obj<AGCObject> at(size_type ix) const;

#ifdef NOT_YET
            /** pretty-printing driver; combine layout+printing **/
            bool pretty(const ppindentinfo & ppii) const;
#endif

            /** first member of list **/
            obj<AGCObject> head_;
            /** remainder of list **/
            DList * rest_ = nullptr;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DList.hpp */
