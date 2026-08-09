/** @file DFloat.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DFloat.hpp"
#include <xo/indentlog/print/pretty.hpp>

namespace xo {
    using xo::facet::typeseq;
    using xo::print::ppdetail_atomic;
    using std::size_t;

    namespace scm {
        DFloat *
        DFloat::_box(obj<AAllocator> mm, double x)
        {
            void * mem = mm.alloc(typeseq::id<DFloat>(),
                                  sizeof(DFloat));

            return new (mem) DFloat(x);
        }

        bool
        DFloat::pretty_deprecated(const ppindentinfo & ppii) const
        {
            return ppdetail_atomic<double>::print_pretty(ppii, value_);
        }

        void
        DFloat::pretty(xo::pp::PpSink & sink) const
        {
            /* leaf, as pretty_deprecated was: ppdetail_atomic is a bare
             * pps()->write(x) with no framing (pretty.hpp:363).
             *
             * NB double formatting is the thing to watch here, not structure --
             * legacy went through ppstate::write(double), the sink through
             * Prettifier<double>.  Pinned by the render test rather than
             * assumed equal.
             */
            sink.pp(value_);
        }

        DFloat *
        DFloat::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DFloat::visit_gco_children(VisitReason, obj<AGCObjectVisitor>) noexcept
        {
            // noop -- childless!
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DFloat.cpp */
