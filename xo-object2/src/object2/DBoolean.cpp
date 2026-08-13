/** @file DBoolean.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DBoolean.hpp"

namespace xo {
    using xo::facet::typeseq;

    namespace scm {
        DBoolean *
        DBoolean::_box(obj<AAllocator> mm, bool x)
        {
            void * mem = mm.alloc(typeseq::id<DBoolean>(),
                                  sizeof(DBoolean));

            return new (mem) DBoolean(x);
        }

        void
        DBoolean::pretty(xo::pp::PpSink & sink) const
        {
            /* leaf, as the deprecated printer was: ppdetail_atomic was a bare
             * pps()->write(x) with no framing (pretty.hpp:363).
             */
            sink.pp(value_ ? "true" : "false");
        }

        DBoolean *
        DBoolean::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DBoolean::visit_gco_children(VisitReason, obj<AGCObjectVisitor>) noexcept
        {
            // no-op. childless
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end DBoolean.cpp */
