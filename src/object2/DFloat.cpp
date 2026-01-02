/** @file DFloat.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "DFloat.hpp"

namespace xo {
    using xo::facet::typeseq;

    namespace scm {
        DFloat *
        DFloat::make(obj<AAllocator> mm,
                     double x)
        {
            void * mem = mm.alloc(typeseq::id<DFloat>(),
                                  sizeof(DFloat));

            return new (mem) DFloat(x);
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end DFloat.cpp */
