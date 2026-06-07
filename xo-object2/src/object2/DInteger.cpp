/** @file DInteger.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DInteger.hpp"
#include <xo/indentlog/print/pretty.hpp>
#include <xo/indentlog/scope.hpp>

namespace xo {
    using xo::facet::typeseq;
    using xo::print::ppdetail_atomic;

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

        bool
        DInteger::pretty(const ppindentinfo & ppii) const
        {
            return ppdetail_atomic<long>::print_pretty(ppii, value_);
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
