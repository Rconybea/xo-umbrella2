/** @file DVsmDefContFrame.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "DVsmDefContFrame.hpp"
#include <xo/expression2/DefineExpr.hpp>
#include <xo/alloc2/Collector.hpp>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::pp::field;
    namespace scm {

        DVsmDefContFrame::DVsmDefContFrame(obj<AGCObject> parent,
                                           VsmInstr cont,
                                           DDefineExpr * def_expr)
                : parent_{parent},
                  cont_{cont},
                  def_expr_{def_expr}
        {}

        DVsmDefContFrame *
        DVsmDefContFrame::make(obj<AAllocator> mm,
                               obj<AGCObject> parent,
                               VsmInstr cont,
                               DDefineExpr * def_expr)
        {
            void * mem = mm.alloc_for<DVsmDefContFrame>();

            return new (mem) DVsmDefContFrame(parent, cont, def_expr);
        }

        // gcobject facet

        DVsmDefContFrame *
        DVsmDefContFrame::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for<DVsmDefContFrame>(this);
        }

        void
        DVsmDefContFrame::visit_gco_children(VisitReason reason,
                                             obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &parent_);
            gc.visit_child(reason, &def_expr_);
        }

        // printable facet

        void
        DVsmDefContFrame::pretty(xo::pp::PpSink & sink) const
        {
            sink.pretty_struct("DVsmDefContFrame", field("cont", cont_));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DVsmDefContFrame.cpp */
