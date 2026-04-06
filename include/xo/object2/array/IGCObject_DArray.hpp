/** @file IGCObject_DArray.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DArray.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DArray.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include "DArray.hpp"

namespace xo { namespace scm { class IGCObject_DArray; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DArray>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DArray,
               xo::scm::IGCObject_DArray>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DArray
         **/
        class IGCObject_DArray {
        public:
            /** @defgroup scm-gcobject-darray-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using ACollector = xo::mm::AGCObject::ACollector;
            using AGCObjectVisitor = xo::mm::AGCObject::AGCObjectVisitor;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-darray-methods **/
            ///@{
            // const methods

            // non-const methods
            /** move instance using allocator **/
            static Opaque shallow_move(DArray & self, obj<ACollector> gc) noexcept;
            /** Invoke fn.visit_child(iface,data) for each child GCObject pointer.
Context: provides address of data pointer so it can be updated in place
when @p fn invokes garbage collector reentry point **/
            static void visit_gco_children(DArray & self, obj<AGCObjectVisitor> fn) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */