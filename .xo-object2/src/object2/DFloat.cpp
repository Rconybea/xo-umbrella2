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
        DFloat::pretty(const ppindentinfo & ppii) const
        {
            return ppdetail_atomic<double>::print_pretty(ppii, value_);
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
