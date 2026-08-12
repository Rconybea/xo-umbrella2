/** @file DClosure.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "Closure.hpp"
#include "LambdaExpr.hpp"
#include "LocalEnv.hpp"
#include "VsmRcx.hpp"
#include <xo/object2/RuntimeError.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/indentlog/scope.hpp>
#include <cstddef>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::mm::AGCObject;
    using xo::print::APrintable;
    using xo::pp::field;

    namespace scm {

        DClosure::DClosure(const DLambdaExpr * lm,
                           const DLocalEnv * env)
        : lambda_{lm}, env_{env}
        {}

        DClosure *
        DClosure::make(obj<AAllocator> mm,
                       const DLambdaExpr * lm,
                       const DLocalEnv * env)
        {
            void * mem = mm.alloc_for<DClosure>();

            return new (mem) DClosure(lm, env);
        }

        obj<AGCObject>
        DClosure::apply_nocheck(obj<ARuntimeContext> rcx,
                                const DArray * args)
        {
            // control here only if you try to invoke a closure
            // as a procedure.
            //
            // May support this later, but requires
            // nesting VSM (because call consumes c++ stack)
            //
            // typically prefer trampoline built into VSM

            (void)args;

            scope log(XO_DEBUG(true));

            auto vsm_rcx
                = obj<ARuntimeContext,DVsmRcx>::from(rcx);

            log && log(xtag("vsm_rcx.data", (void*)vsm_rcx.data()));

            auto err_mm
                = vsm_rcx->error_allocator();

            auto err
                = DRuntimeError::make(err_mm,
                                      "DClosure::apply_nocheck",
                                      "not implemented");
            return err;
        }

        DClosure *
        DClosure::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept {
            return gc.std_move_for(this);
        }

        void
        DClosure::visit_gco_children(VisitReason reason,
                                     obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &lambda_);
            gc.visit_child(reason, &env_);
        }

        // ----- printable facet -----

        bool
        DClosure::pretty_deprecated(const ppindentinfo & ppii) const
        {
            obj<APrintable,DLambdaExpr> lambda_pr(const_cast<DLambdaExpr *>(lambda_));
            obj<APrintable,DLocalEnv> env_pr(const_cast<DLocalEnv *>(env_));

            bool lambda_present = lambda_pr;
            bool env_present = env_pr;

            return ppii.pps()->pretty_struct
                (ppii,
                 "DClosure",
                 refrtag("lambda", lambda_pr, lambda_present),
                 refrtag("env", env_pr, env_present));
        }

        void
        DClosure::pretty(xo::pp::PpSink & sink) const
        {
            obj<APrintable,DLambdaExpr> lambda_pr(const_cast<DLambdaExpr *>(lambda_));
            obj<APrintable,DLocalEnv> env_pr(const_cast<DLocalEnv *>(env_));

            /* TWO per-field present flags, so legacy's flags map straight onto
             * field()'s third argument.  Both children are reached by DIRECT
             * construction of obj<APrintable,T> rather than a registry lookup
             * -- the shape that aborts on use elsewhere -- but here the
             * emptiness is TESTED before the field is emitted, so a null
             * child is an absent field rather than a crash.  That is why
             * DClosure is absent from
             * .xo-backlog/xo-reader2/issues/01-ssm-printer-null-children.md.
             */
            sink.pretty_struct("DClosure",
                               field("lambda", lambda_pr, bool(lambda_pr)),
                               field("env", env_pr, bool(env_pr)));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DClosure.cpp */
