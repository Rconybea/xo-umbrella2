/** @file DList.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DList.hpp"
#include "list/IGCObject_DList.hpp"
#include "list/IPrintable_DList.hpp"
#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>

// need Collector for mm_do_assign()
#include <xo/alloc2/Collector.hpp>
#include <xo/alloc2/gc/RCollector_aux.hpp> // for mm_do_assign()
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/facet/facet_implementation.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/tostr_xx.hpp>

namespace xo {
    /* the ppsink logging/printing vocabulary, for use below */
    using xo::pp::scope;
    using xo::pp::tostr0;
    using xo::pp::xtag;

    using xo::print::APrintable;
    using xo::mm::AGCObject;
    using xo::facet::FacetRegistry;
    //using xo::facet::typeseq;

    namespace scm {
        static DList s_null(obj<AGCObject>(), nullptr);

        DList *
        DList::_nil()
        {
            return &s_null;
        }

        obj<AGCObject,DList>
        DList::nil()
        {
            return obj<AGCObject,DList>(_nil());
        }

        DList *
        DList::_cons(obj<AAllocator> mm,
                     obj<AGCObject> car,
                     DList * cdr)
        {
            void * mem = mm.alloc_for<DList>();

            return new (mem) DList(car, cdr);
        }

        obj<AGCObject,DList>
        DList::cons(obj<AAllocator> mm,
                    obj<AGCObject> car,
                    DList * cdr)
        {
            return obj<AGCObject,DList>(_cons(mm, car, cdr));
        }

#ifdef OBSOLETE
        DList *
        DList::list(obj<AAllocator> mm,
                    obj<AGCObject> h1)
        {
            void * mem = mm.alloc(typeseq::id<DList>(), sizeof(DList));

            return new (mem) DList(h1, DList::_nil());
        }

        DList *
        DList::list(obj<AAllocator> mm,
                    obj<AGCObject> h1,
                    obj<AGCObject> h2)
        {
            void * mem = mm.alloc(typeseq::id<DList>(), sizeof(DList));

            return new (mem) DList(h1, DList::list(mm, h2));
        }
#endif

        bool
        DList::is_empty() const noexcept
        {
            return this == &s_null;
        }

        auto
        DList::size() const noexcept -> size_type
        {
            const DList * l = this;

            size_type z = 0;

            while (l && l != &s_null) {
                ++z;
                l = l->rest_;
            }

            return z;
        }

        auto
        DList::at(size_type index) const -> obj<AGCObject>
        {
            size_type ix = index;
            const DList * l = this;

            while (l->rest_ && (ix > 0)) {
                --ix;
                l = l->rest_;
            }

            if (ix > 0) {
                assert(l == nullptr);

                throw std::runtime_error
                    (tostr0("DList::at: out-of-range index where [0..z) expected",
                           xtag("index", index),
                           xtag("z", this->size())));
            }

            assert(l);

            return l->head_;
        }

        void
        DList::assign_head(obj<AAllocator> mm, obj<AGCObject> rhs)
        {
            scope log(XO_DEBUG_(true), xtag("mm.data", mm.data_));

            mm.barrier_assign(this, &head_, rhs);

            //mm.barrier_assign_aux(this,
            //                      head_.iface(), head_.opaque_data_addr(),
            //                      rhs.iface(), rhs.opaque_data());
        }

        // vestigial. used in MockCollector
        void
        DList::assign_head_gc(obj<ACollector> gc, obj<AGCObject> rhs)
        {
            scope log(XO_DEBUG_(true), xtag("gc.data", gc.data_));

            gc.assign_member(this, &head_, rhs);
        }

        void
        DList::_assign_rest(obj<AAllocator> mm, DList * rest)
        {
            obj<AGCObject,DList> rest_gco(rest);

            mm.barrier_assign_aux(this,
                                  nullptr /*lhs iface unused*/,
                                  (void**)&(this->rest_),
                                  rest_gco.iface(),
                                  rest);
        }

        void
        DList::pretty(xo::pp::PpSink & sink) const
        {
            sink.put("(").begin();

            const DList * l = this;

            size_t i = 0;
            while (!l->is_empty()) {
                if (i > 0)
                    sink.split(1);

                obj<APrintable> elt
                    = FacetRegistry::instance().variant<APrintable, AGCObject>(l->head_);

                assert(elt.data());

                sink.pp(elt);

                l = l->rest_;
                ++i;
            }

            sink.end().put(")");
        }

        // ----- GCObject facet ------

        DList *
        DList::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DList::visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &head_);
            gc.visit_child(reason, &rest_);
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end DList.cpp */
