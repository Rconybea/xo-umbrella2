/** @file IGCObjectVisitor_DGCObjectStoreVisitor.hpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObjectVisitor_DGCObjectStoreVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_repr.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObjectVisitor_DGCObjectStoreVisitor.json5]
 **/

#pragma once

#include "GCObjectVisitor.hpp"
#include "DGCObjectStoreVisitor.hpp"

namespace xo { namespace mm { class IGCObjectVisitor_DGCObjectStoreVisitor; } }

namespace xo {
    namespace facet {
        template <>
        struct FacetImplementation<xo::mm::AGCObjectVisitor,
                                   xo::mm::DGCObjectStoreVisitor>
        {
            using ImplType = xo::mm::IGCObjectVisitor_Xfer
              <xo::mm::DGCObjectStoreVisitor,
               xo::mm::IGCObjectVisitor_DGCObjectStoreVisitor>;
        };
    }
}

namespace xo {
    namespace mm {
        /** @class IGCObjectVisitor_DGCObjectStoreVisitor
         **/
        class IGCObjectVisitor_DGCObjectStoreVisitor {
        public:
            /** @defgroup mm-gcobjectvisitor-dgcobjectstorevisitor-type-traits **/
            ///@{
            using Copaque = xo::mm::AGCObjectVisitor::Copaque;
            using Opaque = xo::mm::AGCObjectVisitor::Opaque;
            ///@}
            /** @defgroup mm-gcobjectvisitor-dgcobjectstorevisitor-methods **/
            ///@{
            // const methods
            /** allocation metadata for gc-aware data at address @p gco.
@p gco must be the result of a call to collector's alloc() function
note: load-bearing for xo-gc/MutationLogStore **/
            static AllocInfo alloc_info(const DGCObjectStoreVisitor & self, void * addr);
            /** generation to which pointer @p addr belongs, given role @p r;
sentinel if @p addr is not owned by collector.
note: load-bearing for xo-gc/MutationLogStore **/
            static Generation generation_of(const DGCObjectStoreVisitor & self, Role r, const void * addr) noexcept;

            // non-const methods
            /** allocate copy of source object at address @p src.
Source must be owned by this collector.
Increments object age **/
            static void * alloc_copy(DGCObjectStoreVisitor & self, std::byte * src);
            /** visit child of a gc-aware object. May update child in-place! **/
            static void visit_child(DGCObjectStoreVisitor & self, VisitReason reason, AGCObject * iface, void ** pp_data) noexcept;
            ///@}
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end */