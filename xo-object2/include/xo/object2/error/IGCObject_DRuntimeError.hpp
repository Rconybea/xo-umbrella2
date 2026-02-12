/** @file IGCObject_DRuntimeError.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DRuntimeError.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DRuntimeError.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include "DRuntimeError.hpp"

namespace xo { namespace scm { class IGCObject_DRuntimeError; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DRuntimeError>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DRuntimeError,
               xo::scm::IGCObject_DRuntimeError>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DRuntimeError
         **/
        class IGCObject_DRuntimeError {
        public:
            /** @defgroup scm-gcobject-druntimeerror-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-druntimeerror-methods **/
            ///@{
            // const methods
            /** memory consumption for this instance **/
            static size_type shallow_size(const DRuntimeError & self) noexcept;
            /** copy instance using allocator **/
            static Opaque shallow_copy(const DRuntimeError & self, obj<AAllocator> mm) noexcept;

            // non-const methods
            /** during GC: forward immdiate children **/
            static size_type forward_children(DRuntimeError & self, obj<ACollector> gc) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */