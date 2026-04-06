/** @file IGCObjectVisitor_DX1Collector.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObjectVisitor_DX1Collector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObjectVisitor_DX1Collector.json5]
 **/

#pragma once

#include "GCObjectVisitor.hpp"
#include "DX1Collector.hpp"

namespace xo { namespace mm { class IGCObjectVisitor_DX1Collector; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObjectVisitor,
                                   xo::mm::DX1Collector>
        {
            using ImplType = xo::mm::IGCObjectVisitor_Xfer
              <xo::mm::DX1Collector,
               xo::mm::IGCObjectVisitor_DX1Collector>;
        };
    }
}

namespace xo {
    namespace mm {
        /** @class IGCObjectVisitor_DX1Collector
         **/
        class IGCObjectVisitor_DX1Collector {
        public:
            /** @defgroup mm-gcobjectvisitor-dx1collector-type-traits **/
            ///@{
            using Copaque = xo::mm::AGCObjectVisitor::Copaque;
            using Opaque = xo::mm::AGCObjectVisitor::Opaque;
            ///@}
            /** @defgroup mm-gcobjectvisitor-dx1collector-methods **/
            ///@{
            // const methods
            /** allocation metadata for gc-aware data at address @p gco.
@p gco must be the result of a call to collector's alloc() function **/
            static AllocInfo alloc_info(const DX1Collector & self, void * addr);

            // non-const methods
            /** allocate copy of source object at address @p src.
Source must be owned by this collector.
Increments object age **/
            static void * alloc_copy(DX1Collector & self, std::byte * src);
            /** visit child of a gc-aware object. May update child in-place! **/
            static void visit_child(DX1Collector & self, AGCObject * iface, void ** pp_data) noexcept;
            ///@}
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end */