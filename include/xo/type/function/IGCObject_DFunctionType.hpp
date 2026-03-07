/** @file IGCObject_DFunctionType.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DFunctionType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DFunctionType.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include "DFunctionType.hpp"

namespace xo { namespace scm { class IGCObject_DFunctionType; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DFunctionType>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DFunctionType,
               xo::scm::IGCObject_DFunctionType>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DFunctionType
         **/
        class IGCObject_DFunctionType {
        public:
            /** @defgroup scm-gcobject-dfunctiontype-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-dfunctiontype-methods **/
            ///@{
            // const methods
            /** memory consumption for this instance **/
            static size_type shallow_size(const DFunctionType & self) noexcept;
            /** copy instance using allocator **/
            static Opaque shallow_copy(const DFunctionType & self, obj<AAllocator> mm) noexcept;

            // non-const methods
            /** during GC: forward immdiate children **/
            static size_type forward_children(DFunctionType & self, obj<ACollector> gc) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */