/** @file DConstant.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "Constant.hpp"
// #include "detail/IExpression_DConstant.hpp"
#include "TypeDescr.hpp"
#include <xo/object2/DFloat.hpp>
#include <xo/object2/DInteger.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/facet/obj.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/pretty_struct.hpp>   /* sink.pretty_struct(..), field(..) */
#include <xo/reflectutil/typeseq.hpp>

namespace xo {
    using xo::scm::DFloat;
    using xo::scm::DInteger;
    using xo::print::APrintable;
    using xo::facet::FacetRegistry;
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;
    using xo::reflect::typeseq;

    namespace scm {
        DConstant::DConstant(obj<AGCObject> value) noexcept
            :
            //typeref_{TypeRef::resolved(td)},
              value_{value}
        {
            // todo: use ObjectConverter here

            auto tseq = value_._typeseq();

            TypeDescr td = this->_lookup_td(tseq);

            if (td) {
                typeref_ = TypeRef::resolved(td);
            }
        }

        obj<AExpression,DConstant>
        DConstant::make(obj<AAllocator> mm,
                        obj<AGCObject> value)
        {
            return obj<AExpression,DConstant>(_make(mm, value));
        }

        DConstant *
        DConstant::_make(obj<AAllocator> mm,
                         obj<AGCObject> value)
        {
            void * mem = mm.alloc(typeseq::id<DConstant>(),
                                  sizeof(DConstant));

            return new (mem) DConstant(value);
        }

        TypeDescr
        DConstant::_lookup_td(typeseq tseq)
        {
            if (tseq == typeseq::id<DFloat>()) {
                /* double */
                return Reflect::require<DFloat::value_type>();
            } else if (tseq == typeseq::id<DInteger>()) {
                /* long */
                return Reflect::require<DInteger::value_type>();
            }

            return nullptr;
        }

        DConstant *
        DConstant::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DConstant::visit_gco_children(VisitReason reason,
                                      obj<AGCObjectVisitor> gc) noexcept
        {
            typeref_.visit_gco_children(reason, gc);

            gc.visit_child(reason, &value_);
        }

        void
        DConstant::pretty(xo::pp::PpSink & sink) const
        {
            /* named locals: field() captures BY REFERENCE (pretty_struct.hpp),
             * and both typeseqs are temporaries returned by value.
             */
            obj<APrintable> value_pr
                = FacetRegistry::instance().variant<APrintable,AGCObject>(value_);

            /* typeseq has no Prettifier<> and no ppdetail<> -- only an
             * operator<< (xo-reflectutil/typeseq.hpp:115), so these two fields
             * take ppsink's leaf fallback, as Binding does in DVarRef.
             * Rendering is the bare seqno either way.
             */
            const typeseq data_tseq = value_._typeseq();
            const typeseq facet_tseq = value_pr._typeseq();

            sink.pretty_struct("DConstant",
                               xo::pp::field("value_.tseq", data_tseq),
                               xo::pp::field("value.tseq", facet_tseq),
                               xo::pp::field("value", value_pr));
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end DConstant.cpp */
