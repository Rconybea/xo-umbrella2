/** @file IGCObject_DList.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DList.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DList.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include "DList.hpp"

namespace xo { namespace scm { class IGCObject_DList; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DList>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DList,
               xo::scm::IGCObject_DList>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DList
         **/
        class IGCObject_DList {
        public:
            /** @defgroup scm-gcobject-dlist-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-dlist-methods **/
            ///@{
            // const methods

            // non-const methods
            /** move instance using allocator **/
            static Opaque shallow_move(DList & self, obj<ACollector> gc) noexcept;
            /** during GC: forward immdiate children **/
            static void forward_children(DList & self, obj<ACollector> gc) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */