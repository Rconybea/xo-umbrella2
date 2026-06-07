/** @file DSimpleRcx.cpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#include "DSimpleRcx.hpp"
#include <xo/alloc2/Collector.hpp>
#include <xo/facet/FacetRegistry.hpp>

namespace xo {
    using xo::mm::ACollector;

    namespace scm {

        obj<ACollector>
        DSimpleRcx::collector() const noexcept
        {
            return allocator_.try_to_facet<ACollector>();
        }

        void
        DSimpleRcx::visit_pools(const MemorySizeVisitor & visitor) const
        {
            allocator_.visit_pools(visitor);
        }
    }
}

/* end DSimpleRcx.cpp */
