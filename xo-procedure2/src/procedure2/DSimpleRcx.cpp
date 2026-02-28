/** @file DSimpleRcx.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "DSimpleRcx.hpp"

namespace xo {
    namespace scm {

        void
        DSimpleRcx::visit_pools(const MemorySizeVisitor & visitor) const
        {
            allocator_.visit_pools(visitor);
        }
    }
}

/* end DSimpleRcx.cpp */
