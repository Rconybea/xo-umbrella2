/** @file DFloat.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DFloat.hpp"
#include <xo/indentlog/print/pretty.hpp>

namespace xo {
    using xo::facet::typeseq;
    using xo::print::ppdetail_atomic;

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
    } /*namespace scm*/
} /*namespace xo*/

/* end DFloat.cpp */
