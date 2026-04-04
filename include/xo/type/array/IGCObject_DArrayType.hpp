/** @file IGCObject_DArrayType.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DArrayType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DArrayType.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include "DArrayType.hpp"

namespace xo { namespace scm { class IGCObject_DArrayType; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DArrayType>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DArrayType,
               xo::scm::IGCObject_DArrayType>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DArrayType
         **/
        class IGCObject_DArrayType {
        public:
            /** @defgroup scm-gcobject-darraytype-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-darraytype-methods **/
            ///@{
            // const methods
            /** memory consumption for this instance **/
            static size_type shallow_size(const DArrayType & self) noexcept;

            // non-const methods
            /** move instance using allocator **/
            static Opaque shallow_move(DArrayType & self, obj<AAllocator> mm) noexcept;
            /** during GC: forward immdiate children **/
            static size_type forward_children(DArrayType & self, obj<ACollector> gc) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */