/** @file DVsmApplyFrame.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "DVsmApplyFrame.hpp"
#include <xo/object2/Array.hpp>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::pp::field;
    using xo::facet::typeseq;

    namespace scm {

        DVsmApplyFrame::DVsmApplyFrame(obj<AGCObject> old_parent,
                                       VsmInstr old_cont,
                                       DArray * args)
            : parent_{old_parent},
              cont_{old_cont},
              args_{args}
        {}

        DVsmApplyFrame *
        DVsmApplyFrame::make(obj<AAllocator> mm,
                             obj<AGCObject> old_parent,
                             VsmInstr old_cont,
                             DArray * args)
        {
            DVsmApplyFrame * result = nullptr;

            void * mem = mm.alloc(typeseq::id<DVsmApplyFrame>(),
                                  sizeof(DVsmApplyFrame));

            result = new (mem) DVsmApplyFrame(old_parent,
                                              old_cont,
                                              args);

            assert(result);

            return result;
        }

        DVsmApplyFrame *
        DVsmApplyFrame::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DVsmApplyFrame::visit_gco_children(VisitReason reason,
                                           obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &parent_);
            gc.visit_child(reason, &fn_);
            gc.visit_child(reason, &args_);
        }

        void
        DVsmApplyFrame::pretty(xo::pp::PpSink & sink) const
        {
            /* named local: field() captures BY REFERENCE and size() returns
             * by value.
             *
             * args_ dereferenced unguarded, exactly as legacy does.  Whether
             * it can be null here is UNVERIFIED -- recorded in
             * .xo-backlog/xo-reader2/issues/01-ssm-printer-null-children.md
             * alongside DLocalEnv, which has the identical shape and where
             * the factory demonstrably does not check.
             */
            const auto n_args = args_->size();

            sink.pretty_struct("DVsmApplyFrame",
                               field("cont", cont_),
                               field("n_args", n_args));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DVsmApplyFrame.cpp */
