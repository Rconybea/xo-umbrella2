/** @file DList.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/gc/GCObject.hpp>
#include <xo/facet/obj.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>

namespace xo {
    namespace scm {

        // TODO: consider renaming to DCons
        // See also ListOps in ListOps.hpp
        //
        struct DList {
            using size_type = std::size_t;
            using AGCObject = xo::mm::AGCObject;
            using AAllocator = xo::mm::AAllocator;
            using ppindentinfo = xo::print::ppindentinfo;

            DList(xo::obj<AGCObject> h,
                  DList * r) : head_{h}, rest_{r} {}

            /** sentinel for null list **/
            static DList * _nil();

            /** list with first element @p car,
             *  followed by contents of list @p cdr.
             *  Shares structure with @p cdr
             **/
            static DList * _cons(obj<AAllocator> mm,
                                 obj<AGCObject> car,
                                 DList * cdr);

#ifdef OBSOLETE
            /** list with one element @p h1, allocated from @p mm **/
            static DList * list(obj<AAllocator> mm,
                                obj<AGCObject> h1);
            /** list with two elements @p h1, @p h2, allocated from @p mm **/
            static DList * list(obj<AAllocator> mm,
                                obj<AGCObject> h1,
                                obj<AGCObject> h2);
#endif

            /** DList length is at least 1 **/
            bool is_empty() const noexcept;
            /** DList models a finite sequence **/
            bool is_finite() const noexcept { return true; };
            /** return number of elements in this DList **/
            size_type size() const noexcept;
            /** return element at 0-based index @p ix **/
            obj<AGCObject> at(size_type ix) const;

            /** pretty-printing driver; combine layout+printing **/
            bool pretty(const ppindentinfo & ppii) const;

            /** first member of list **/
            obj<AGCObject> head_;
            /** remainder of list **/
            DList * rest_ = nullptr;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DList.hpp */
