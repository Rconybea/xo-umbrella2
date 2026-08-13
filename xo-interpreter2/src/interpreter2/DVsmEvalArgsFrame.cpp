/** @file DVsmEvalArgsFrame.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "DVsmEvalArgsFrame.hpp"
#include <xo/expression2/ApplyExpr.hpp>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::pp::field;
    using xo::facet::typeseq;

    namespace scm {

        // ----- VsmEvalArgsFrame -----

        DVsmEvalArgsFrame::DVsmEvalArgsFrame(DVsmApplyFrame * parent,
                                             VsmInstr cont,
                                             DApplyExpr * apply_expr)
        : parent_{parent},
          cont_{cont},
          apply_expr_{apply_expr}
        {}

        DVsmEvalArgsFrame *
        DVsmEvalArgsFrame::make(obj<AAllocator> mm,
                                DVsmApplyFrame * apply_frame,
                                VsmInstr cont,
                                DApplyExpr * apply_expr)
        {
            DVsmEvalArgsFrame * result = nullptr;

            void * mem = mm.alloc(typeseq::id<DVsmEvalArgsFrame>(),
                                  sizeof(DVsmEvalArgsFrame));

            result = new (mem) DVsmEvalArgsFrame(apply_frame, cont, apply_expr);

            assert(result);

            return result;
        }

        DVsmEvalArgsFrame *
        DVsmEvalArgsFrame::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DVsmEvalArgsFrame::visit_gco_children(VisitReason reason,
                                              obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &parent_);
            gc.visit_child(reason, &apply_expr_);
        }

        void
        DVsmEvalArgsFrame::pretty(xo::pp::PpSink & sink) const
        {
            sink.pretty_struct("DVsmEvalArgsFrame",
                               field("cont", cont_),
                               field("i_arg", i_arg_));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end VsmEvalArgsFrame.cpp */
