/** @file IRuntimeContext_DSimpleRcx.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IRuntimeContext_DSimpleRcx.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IRuntimeContext_DSimpleRcx.json5]
 **/

#pragma once

#include "RuntimeContext.hpp"
#include "DSimpleRcx.hpp"

namespace xo { namespace scm { class IRuntimeContext_DSimpleRcx; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::scm::ARuntimeContext,
                                   xo::scm::DSimpleRcx>
        {
            using ImplType = xo::scm::IRuntimeContext_Xfer
              <xo::scm::DSimpleRcx,
               xo::scm::IRuntimeContext_DSimpleRcx>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IRuntimeContext_DSimpleRcx
         **/
        class IRuntimeContext_DSimpleRcx {
        public:
            /** @defgroup scm-runtimecontext-dsimplercx-type-traits **/
            ///@{
            using AAllocator = xo::scm::ARuntimeContext::AAllocator;
            using Copaque = xo::scm::ARuntimeContext::Copaque;
            using Opaque = xo::scm::ARuntimeContext::Opaque;
            ///@}
            /** @defgroup scm-runtimecontext-dsimplercx-methods **/
            ///@{
            // const methods
            /** default allocator to use for objects **/
            static obj<AAllocator> allocator(const DSimpleRcx & self) noexcept;

            // non-const methods
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */