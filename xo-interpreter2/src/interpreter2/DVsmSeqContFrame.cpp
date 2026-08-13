/** @file DVsmSeqContFrame.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "DVsmSeqContFrame.hpp"
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::pp::field;
    namespace scm {

        DVsmSeqContFrame::DVsmSeqContFrame(obj<AGCObject> parent,
                                           VsmInstr cont,
                                           DSequenceExpr * seq_expr,
                                           uint32_t i_seq)
                : parent_{parent},
                  cont_{cont},
                  seq_expr_{seq_expr},
                  i_seq_{i_seq}
        {}

        DVsmSeqContFrame *
        DVsmSeqContFrame::make(obj<AAllocator> mm,
                                    obj<AGCObject> parent,
                                    VsmInstr cont,
                                    DSequenceExpr * seq_expr,
                                    uint32_t i_seq)
        {
            void * mem = mm.alloc_for<DVsmSeqContFrame>();

            return new (mem) DVsmSeqContFrame(parent, cont, seq_expr, i_seq);
        }

        // gcobject facet

        DVsmSeqContFrame *
        DVsmSeqContFrame::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DVsmSeqContFrame::visit_gco_children(VisitReason reason,
                                             obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &parent_);
            gc.visit_child(reason, &seq_expr_);
        }

        // printable facet

        void
        DVsmSeqContFrame::pretty(xo::pp::PpSink & sink) const
        {
            sink.pretty_struct("DVsmSeqContFrame",
                               field("cont", cont_),
                               field("i_seq", i_seq_));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DVsmSeqContFrame.cpp */
