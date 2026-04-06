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
            using AGCObjectVisitor = xo::mm::AGCObject::AGCObjectVisitor;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-dtypevarref-methods **/
            ///@{
            // const methods

            // non-const methods
            /** move instance using allocator **/
            static Opaque shallow_move(DTypeVarRef & self, obj<ACollector> gc) noexcept;
            /** Invoke fn.visit_child(iface,data) for each child GCObject pointer.
Context: provides address of data pointer so it can be updated in place
when @p fn invokes garbage collector reentry point **/
            static void visit_gco_children(DTypeVarRef & self, obj<AGCObjectVisitor> fn) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */