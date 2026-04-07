/** @file IGCObjectVisitor_DX1Collector.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObjectVisitor_DX1Collector.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObjectVisitor_DX1Collector.json5]
**/

#include "detail/IGCObjectVisitor_DX1Collector.hpp"

namespace xo {
    namespace mm {
        auto
        IGCObjectVisitor_DX1Collector::alloc_info(const DX1Collector & self, void * addr) -> AllocInfo
        {
            return self.alloc_info(addr);
        }

        auto
        IGCObjectVisitor_DX1Collector::generation_of(const DX1Collector & self, Role r, const void * addr) noexcept -> Generation
        {
            return self.generation_of(r, addr);
        }

        auto
        IGCObjectVisitor_DX1Collector::alloc_copy(DX1Collector & self, std::byte * src) -> void *
        {
            return self.alloc_copy(src);
        }
        auto
        IGCObjectVisitor_DX1Collector::visit_child(DX1Collector & self, AGCObject * iface, void ** pp_data) noexcept -> void
        {
            self.visit_child(iface, pp_data);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end IGCObjectVisitor_DX1Collector.cpp */
