/** @file DVsmApplyClosureFrame.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "DVsmApplyClosureFrame.hpp"
#include "LocalEnv.hpp"
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::mm::AGCObject;
    using xo::reflect::typeseq;
    using xo::pp::field;

    namespace scm {

        DVsmApplyClosureFrame::DVsmApplyClosureFrame(obj<AGCObject> stack,
                                                     VsmInstr cont,
                                                     DLocalEnv * local_env)
            : stack_{stack},
              cont_{cont},
              local_env_{local_env}
        {}

        DVsmApplyClosureFrame *
        DVsmApplyClosureFrame::make(obj<AAllocator> mm,
                                    obj<AGCObject> stack,
                                    VsmInstr cont,
                                    DLocalEnv * local_env)
        {
            void * mem = mm.alloc(typeseq::id<DVsmApplyClosureFrame>(),
                                  sizeof(DVsmApplyClosureFrame));

            return new (mem) DVsmApplyClosureFrame(stack, cont, local_env);
        }

        DVsmApplyClosureFrame *
        DVsmApplyClosureFrame::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DVsmApplyClosureFrame::visit_gco_children(VisitReason reason,
                                                  obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &stack_);
            gc.visit_child(reason, &local_env_);
        }

        bool
        DVsmApplyClosureFrame::pretty_deprecated(const ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct
                (ppii,
                 "DVsmApplyClosureFrame",
                 refrtag("cont", cont_),
                 refrtag("env", local_env_));
        }

        void
        DVsmApplyClosureFrame::pretty(xo::pp::PpSink & sink) const
        {
            sink.pretty_struct("DVsmApplyClosureFrame",
                               field("cont", cont_),
                               field("env", local_env_));
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end DVsmApplyClosureFrame.cpp */
