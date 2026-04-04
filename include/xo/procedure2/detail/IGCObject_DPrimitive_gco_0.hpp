/** @file IGCObject_DPrimitive_gco_0.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DPrimitive_gco_0.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DPrimitive_gco_0.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>
#include "DPrimitive_gco_0.hpp"

namespace xo { namespace scm { class IGCObject_DPrimitive_gco_0; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DPrimitive_gco_0>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DPrimitive_gco_0,
               xo::scm::IGCObject_DPrimitive_gco_0>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DPrimitive_gco_0
         **/
        class IGCObject_DPrimitive_gco_0 {
        public:
            /** @defgroup scm-gcobject-dprimitive_gco_0-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-dprimitive_gco_0-methods **/
            ///@{
            // const methods
            /** memory consumption for this instance **/
            static size_type shallow_size(const DPrimitive_gco_0 & self) noexcept;

            // non-const methods
            /** move instance using allocator **/
            static Opaque shallow_move(DPrimitive_gco_0 & self, obj<ACollector> gc) noexcept;
            /** during GC: forward immdiate children **/
            static void forward_children(DPrimitive_gco_0 & self, obj<ACollector> gc) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */