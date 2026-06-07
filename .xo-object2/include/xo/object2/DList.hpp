/** @file DList.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/GCObject.hpp>
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
            using ACollector = xo::mm::ACollector;
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;
            using ppindentinfo = xo::print::ppindentinfo;

        public:
            DList(xo::obj<AGCObject> h,
                  DList * r) : head_{h}, rest_{r} {}

            /** sentinel for null list.  Idempotent.
             *  Application code may prefer ListOps::nil()
             **/
            static DList * _nil();

            /** like _nil(), but retrn fop wrapper **/
            static obj<AGCObject,DList> nil();

            /** list with first element @p car,
             *  followed by contents of list @p cdr.
             *  Shares structure with @p cdr
             *  Application code may prefer ListOps::cons()
             **/
            static DList * _cons(obj<AAllocator> mm,
                                 obj<AGCObject> car,
                                 DList * cdr);

            /** like @c _cons(mm,car,cdr), but return fop wrapper **/
            static obj<AGCObject,DList> cons(obj<AAllocator> mm,
                                             obj<AGCObject> car,
                                             DList * cdr);

            obj<AGCObject> head() const noexcept { return head_; }
            DList * rest() const noexcept { return rest_; }

            /** DList length is at least 1 **/
            bool is_empty() const noexcept;
            /** DList models a finite sequence **/
            bool is_finite() const noexcept { return true; };
            /** return number of elements in this DList **/
            size_type size() const noexcept;
            /** return element at 0-based index @p ix **/
            obj<AGCObject> at(size_type ix) const;

            /** assign head **/
            void assign_head(obj<AAllocator> mm, obj<AGCObject> h);
            void assign_head_gc(obj<ACollector> gc, obj<AGCObject> h);
            /** assign rest-pointer. Caller responsible for preserving acyclic property! **/
            void _assign_rest(obj<AAllocator> mm, DList * r);

            /** pretty-printing driver; combine layout+printing **/
            bool pretty(const ppindentinfo & ppii) const;

            // GCObject facet

            /** @defgroup xo-scm-list-gcobject-facet gcobject facet **/
            ///@{
            DList * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;
            ///@}

            /** first member of list **/
            obj<AGCObject> head_;
            /** remainder of list **/
            DList * rest_ = nullptr;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DList.hpp */
