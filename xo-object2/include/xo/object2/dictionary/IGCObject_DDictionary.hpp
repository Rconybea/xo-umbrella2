/** @file IGCObject_DDictionary.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DDictionary.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DDictionary.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include "DDictionary.hpp"

namespace xo { namespace scm { class IGCObject_DDictionary; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DDictionary>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DDictionary,
               xo::scm::IGCObject_DDictionary>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DDictionary
         **/
        class IGCObject_DDictionary {
        public:
            /** @defgroup scm-gcobject-ddictionary-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-ddictionary-methods **/
            ///@{
            // const methods
            /** memory consumption for this instance **/
            static size_type shallow_size(const DDictionary & self) noexcept;

            // non-const methods
            /** move instance using allocator **/
            static Opaque shallow_move(DDictionary & self, obj<ACollector> gc) noexcept;
            /** during GC: forward immdiate children **/
            static size_type forward_children(DDictionary & self, obj<ACollector> gc) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */