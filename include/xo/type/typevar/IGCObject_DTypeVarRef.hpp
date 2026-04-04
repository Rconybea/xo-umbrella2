/** @file IGCObject_DTypeVarRef.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DTypeVarRef.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DTypeVarRef.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include "DTypeVarRef.hpp"

namespace xo { namespace scm { class IGCObject_DTypeVarRef; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DTypeVarRef>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DTypeVarRef,
               xo::scm::IGCObject_DTypeVarRef>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DTypeVarRef
         **/
        class IGCObject_DTypeVarRef {
        public:
            /** @defgroup scm-gcobject-dtypevarref-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-dtypevarref-methods **/
            ///@{
            // const methods
            /** memory consumption for this instance **/
            static size_type shallow_size(const DTypeVarRef & self) noexcept;

            // non-const methods
            /** copy instance using allocator **/
            static Opaque shallow_copy(DTypeVarRef & self, obj<AAllocator> mm) noexcept;
            /** during GC: forward immdiate children **/
            static size_type forward_children(DTypeVarRef & self, obj<ACollector> gc) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */