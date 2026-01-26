/** @file IGCObject_DPrimitive_gco_2_gco_gco.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/home/roland/proj/xo-umbrella2-claude1/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DPrimitive_gco_2_gco_gco.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DPrimitive_gco_2_gco_gco.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include <xo/gc/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>
#include "DPrimitive_gco_2_gco_gco.hpp"

namespace xo { namespace scm { class IGCObject_DPrimitive_gco_2_gco_gco; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DPrimitive_gco_2_gco_gco>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DPrimitive_gco_2_gco_gco,
               xo::scm::IGCObject_DPrimitive_gco_2_gco_gco>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DPrimitive_gco_2_gco_gco
         **/
        class IGCObject_DPrimitive_gco_2_gco_gco {
        public:
            /** @defgroup scm-gcobject-dprimitive_gco_2_gco_gco-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-dprimitive_gco_2_gco_gco-methods **/
            ///@{
            // const methods
            /** memory consumption for this instance **/
            static size_type shallow_size(const DPrimitive_gco_2_gco_gco & self) noexcept;
            /** copy instance using allocator **/
            static Opaque shallow_copy(const DPrimitive_gco_2_gco_gco & self, obj<AAllocator> mm) noexcept;

            // non-const methods
            /** during GC: forward immdiate children **/
            static size_type forward_children(DPrimitive_gco_2_gco_gco & self, obj<ACollector> gc) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */