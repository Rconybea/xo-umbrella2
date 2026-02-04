/** @file IRuntimeContext_DSimpleRcx.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IRuntimeContext_DSimpleRcx.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IRuntimeContext_DSimpleRcx.json5]
**/

#include "detail/IRuntimeContext_DSimpleRcx.hpp"

namespace xo {
    namespace scm {
        auto
        IRuntimeContext_DSimpleRcx::allocator(const DSimpleRcx & self) noexcept -> obj<AAllocator>
        {
            return self.allocator();
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end IRuntimeContext_DSimpleRcx.cpp */
