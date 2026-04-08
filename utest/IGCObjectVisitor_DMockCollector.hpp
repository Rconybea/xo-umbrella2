/** @file IGCObjectVisitor_DMockCollector.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObjectVisitor_DMockCollector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObjectVisitor_DMockCollector.json5]
 **/

#pragma once

#include "GCObjectVisitor.hpp"
#include "DMockCollector.hpp"

namespace xo { namespace mm { class IGCObjectVisitor_DMockCollector; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObjectVisitor,
                                   xo::mm::DMockCollector>
        {
            using ImplType = xo::mm::IGCObjectVisitor_Xfer
              <xo::mm::DMockCollector,
               xo::mm::IGCObjectVisitor_DMockCollector>;
        };
    }
}

namespace xo {
    namespace mm {
        /** @class IGCObjectVisitor_DMockCollector
         **/
        class IGCObjectVisitor_DMockCollector {
        public:
            /** @defgroup mm-gcobjectvisitor-dmockcollector-type-traits **/
            ///@{
            using Copaque = xo::mm::AGCObjectVisitor::Copaque;
            using Opaque = xo::mm::AGCObjectVisitor::Opaque;
            ///@}
            /** @defgroup mm-gcobjectvisitor-dmockcollector-methods **/
            ///@{
            // const methods
            /** allocation metadata for gc-aware data at address @p gco.
@p gco must be the result of a call to collector's alloc() function **/
            static AllocInfo alloc_info(const DMockCollector & self, void * addr);
            /** generation to which pointer @p addr belongs, given role @p r;
sentinel if @p addr is not owned by collector **/
            static Generation generation_of(const DMockCollector & self, Role r, const void * addr) noexcept;

            // non-const methods
            /** allocate copy of source object at address @p src.
Source must be owned by this collector.
Increments object age **/
            static void * alloc_copy(DMockCollector & self, std::byte * src);
            /** visit child of a gc-aware object. May update child in-place! **/
            static void visit_child(DMockCollector & self, AGCObject * iface, void ** pp_data) noexcept;
            ///@}
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end */