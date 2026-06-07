/** @file IGCObjectVisitor_DGCObjectStoreVisitor.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObjectVisitor_DGCObjectStoreVisitor.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObjectVisitor_DGCObjectStoreVisitor.json5]
**/

#include "detail/IGCObjectVisitor_DGCObjectStoreVisitor.hpp"

namespace xo {
    namespace mm {
        auto
        IGCObjectVisitor_DGCObjectStoreVisitor::alloc_info(const DGCObjectStoreVisitor & self, void * addr) -> AllocInfo
        {
            return self.alloc_info(addr);
        }

        auto
        IGCObjectVisitor_DGCObjectStoreVisitor::generation_of(const DGCObjectStoreVisitor & self, Role r, const void * addr) noexcept -> Generation
        {
            return self.generation_of(r, addr);
        }

        auto
        IGCObjectVisitor_DGCObjectStoreVisitor::alloc_copy(DGCObjectStoreVisitor & self, std::byte * src) -> void *
        {
            return self.alloc_copy(src);
        }
        auto
        IGCObjectVisitor_DGCObjectStoreVisitor::visit_child(DGCObjectStoreVisitor & self, VisitReason reason, AGCObject * iface, void ** pp_data) noexcept -> void
        {
            self.visit_child(reason, iface, pp_data);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end IGCObjectVisitor_DGCObjectStoreVisitor.cpp */
