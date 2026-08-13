/** @file DInteger.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DInteger.hpp"

namespace xo {
    using xo::facet::typeseq;

    namespace scm {
        DInteger *
        DInteger::_box(obj<AAllocator> mm, long x)
        {
            void * mem = mm.alloc(typeseq::id<DInteger>(),
                                  sizeof(DInteger));

            if (mem)
                return new (mem) DInteger(x);

            return nullptr;
        }

        void
        DInteger::pretty(xo::pp::PpSink & sink) const
        {
            /* ppdetail_atomic<T>::print_pretty is a bare pps()->write(x) -- a
             * leaf with no framing (pretty.hpp:363).  Same on the sink side.
             */
            sink.pp(value_);
        }

        DInteger *
        DInteger::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DInteger::visit_gco_children(VisitReason, obj<AGCObjectVisitor>) noexcept
        {
            // no-op.  childless
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DInteger.cpp */
