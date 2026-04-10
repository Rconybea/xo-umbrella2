/** @file IGCObjectVisitor_DMockCollector.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObjectVisitor_DMockCollector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObjectVisitor_DMockCollector.json5]
**/

#include "./IGCObjectVisitor_DMockCollector.hpp"

namespace xo {
    namespace mm {
        auto
        IGCObjectVisitor_DMockCollector::alloc_info(const DMockCollector & self, void * addr) -> AllocInfo
        {
            return self.alloc_info(addr);
        }

        auto
        IGCObjectVisitor_DMockCollector::generation_of(const DMockCollector & self, Role r, const void * addr) noexcept -> Generation
        {
            return self.generation_of(r, addr);
        }

        auto
        IGCObjectVisitor_DMockCollector::alloc_copy(DMockCollector & self, std::byte * src) -> void *
        {
            return self.alloc_copy(src);
        }
        auto
        IGCObjectVisitor_DMockCollector::visit_child(DMockCollector & self, VisitReason reason, AGCObject * iface, void ** pp_data) noexcept -> void
        {
            self.visit_child(reason, iface, pp_data);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end IGCObjectVisitor_DMockCollector.cpp */
