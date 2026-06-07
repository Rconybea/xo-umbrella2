/** @file IGCObject_DPrimitive_gco_3_dict_string_gco.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DPrimitive_gco_3_dict_string_gco.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DPrimitive_gco_3_dict_string_gco.json5]
 **/

#pragma once

#include "GCObject.hpp"
#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/Allocator.hpp>
#include "DPrimitive_gco_3_dict_string_gco.hpp"

namespace xo { namespace scm { class IGCObject_DPrimitive_gco_3_dict_string_gco; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObject,
                                   xo::scm::DPrimitive_gco_3_dict_string_gco>
        {
            using ImplType = xo::mm::IGCObject_Xfer
              <xo::scm::DPrimitive_gco_3_dict_string_gco,
               xo::scm::IGCObject_DPrimitive_gco_3_dict_string_gco>;
        };
    }
}

namespace xo {
    namespace scm {
        /** @class IGCObject_DPrimitive_gco_3_dict_string_gco
         **/
        class IGCObject_DPrimitive_gco_3_dict_string_gco {
        public:
            /** @defgroup scm-gcobject-dprimitive_gco_3_dict_string_gco-type-traits **/
            ///@{
            using size_type = xo::mm::AGCObject::size_type;
            using AAllocator = xo::mm::AGCObject::AAllocator;
            using AGCObjectVisitor = xo::mm::AGCObject::AGCObjectVisitor;
            using VisitReason = xo::mm::AGCObject::VisitReason;
            using Copaque = xo::mm::AGCObject::Copaque;
            using Opaque = xo::mm::AGCObject::Opaque;
            ///@}
            /** @defgroup scm-gcobject-dprimitive_gco_3_dict_string_gco-methods **/
            ///@{
            // const methods

            // non-const methods
            /** move instance using object visitor.
Arguably abusing the word 'visitor' here **/
            static Opaque gco_shallow_move(DPrimitive_gco_3_dict_string_gco & self, obj<AGCObjectVisitor> gc) noexcept;
            /** Invoke fn.visit_child(iface,data) for each child GCObject pointer.
Context: provides address of data pointer so it can be updated in place
when @p fn invokes garbage collector reentry point **/
            static void visit_gco_children(DPrimitive_gco_3_dict_string_gco & self, VisitReason reason, obj<AGCObjectVisitor> fn) noexcept;
            ///@}
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end */