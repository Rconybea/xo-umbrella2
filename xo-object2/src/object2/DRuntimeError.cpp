/** @file DRuntimeError.cpp
*
 *  @author Roland Conybeare, Feb 2026
 **/

#include "RuntimeError.hpp"
#include <xo/indentlog/print/pretty.hpp>  /* pretty_deprecated body only; DELETE AT PHASE E.
                                          * Was arriving transitively via
                                          * ppdetail_Printable.hpp, which phase E removes. */
#include <xo/ppsink/pretty_struct.hpp>   /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::print::APrintable;
    using xo::mm::AGCObject;
    using xo::facet::typeseq;

    namespace scm {

        obj<AGCObject,DRuntimeError>
        DRuntimeError::make(obj<AAllocator> mm,
                            const char * src_fn,
                            const char * error_descr)
        {
            DRuntimeError * err = DRuntimeError::_make(mm, nullptr, nullptr);

            // pedantic: allocate strings after allocating DRuntimeError instance

            DString * src = DString::from_cstr(mm, src_fn);
            DString * err_descr = DString::from_cstr(mm, error_descr);

            err->src_function_ = src;
            err->error_descr_ = err_descr;

            return obj<AGCObject,DRuntimeError>(err);
        }

        DRuntimeError *
        DRuntimeError::_make(obj<AAllocator> mm,
                             DString * src_fn,
                             DString * error_descr)
        {
            void * mem
                = mm.alloc(typeseq::id<DRuntimeError>(),
                           sizeof(DRuntimeError));

            DRuntimeError * err
                = new (mem) DRuntimeError(src_fn, error_descr);

            return err;
        }

        DRuntimeError::DRuntimeError(DString * src_fn,
                                     DString * error_descr) : src_function_{src_fn},
                                                              error_descr_{error_descr}
        {}

        // ----- GCObject facet -----

        DRuntimeError *
        DRuntimeError::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DRuntimeError::visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &src_function_);
            gc.visit_child(reason, &error_descr_);
        }

        // ----- Printable facet -----

        bool
        DRuntimeError::pretty_deprecated(const ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct
                (ppii,
                 "DRuntimeError",
                 refrtag("src", obj<APrintable,DString>(src_function_)),
                 refrtag("err", obj<APrintable,DString>(error_descr_)));
        }

        void
        DRuntimeError::pretty(xo::pp::PpSink & sink) const
        {
            /* named locals, not temporaries in the call: xo::pp::field captures
             * BY REFERENCE (pretty_struct.hpp:52).  A prvalue argument would in
             * fact survive to the end of the full-expression, but the header
             * asks callers not to rely on that.
             */
            obj<APrintable,DString> src(src_function_);
            obj<APrintable,DString> err(error_descr_);

            sink.pretty_struct("DRuntimeError",
                               xo::pp::field("src", src),
                               xo::pp::field("err", err));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DRuntimeError.cpp */
