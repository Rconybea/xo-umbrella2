/** @file IGCObject_DAtomicType.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DAtomicType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DAtomicType.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include "DAtomicType.hpp"

namespace xo { namespace scm { class IGCObject_DAtomicType; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DAtomicType>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DAtomicType,
               xo::scm::IGCObject_DAtomicType>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DAtomicType
         **/
        class IGCObject_DAtomicType {
        public:
            /** @defgroup scm-gcobject-datomictype-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-datomictype-methods **/
            ///@{
            // const methods
            /** memory consumption for this instance **/
            static size_type shallow_size(const DAtomicType & self) noexcept;

            // non-const methods
            /** move instance using allocator **/
            static Opaque shallow_move(DAtomicType & self, obj<ACollector> gc) noexcept;
            /** during GC: forward immdiate children **/
            static void forward_children(DAtomicType & self, obj<ACollector> gc) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */