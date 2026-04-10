/** @file IGCObject_DPrimitive_gco_1_gco.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DPrimitive_gco_1_gco.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DPrimitive_gco_1_gco.json5]
**/

#include "detail/IGCObject_DPrimitive_gco_1_gco.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DPrimitive_gco_1_gco::gco_shallow_move(DPrimitive_gco_1_gco & self, obj<AGCObjectVisitor> gc) noexcept -> Opaque
        {
            return self.gco_shallow_move(gc);
        }
        auto
        IGCObject_DPrimitive_gco_1_gco::visit_gco_children(DPrimitive_gco_1_gco & self, VisitReason reason, obj<AGCObjectVisitor> fn) noexcept -> void
        {
            self.visit_gco_children(reason, fn);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DPrimitive_gco_1_gco.cpp */
