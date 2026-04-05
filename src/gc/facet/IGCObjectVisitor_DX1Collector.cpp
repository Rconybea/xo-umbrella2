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
        IGCObjectVisitor_DX1Collector::visit_child(DX1Collector & self, AGCObject * iface, void ** pp_data) noexcept -> void
        {
            self.visit_child(iface, pp_data);
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end IGCObjectVisitor_DX1Collector.cpp */
