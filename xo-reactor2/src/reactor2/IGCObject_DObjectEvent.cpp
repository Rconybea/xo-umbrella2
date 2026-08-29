/** @file IGCObject_DObjectEvent.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DObjectEvent.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DObjectEvent.json5]
**/

#include "event/IGCObject_DObjectEvent.hpp"

namespace xo {
    namespace process {
        auto
        IGCObject_DObjectEvent::gco_shallow_move(DObjectEvent & self, obj<AGCObjectVisitor> gc) noexcept -> Opaque
        {
            return self.gco_shallow_move(gc);
        }
        auto
        IGCObject_DObjectEvent::visit_gco_children(DObjectEvent & self, VisitReason reason, obj<AGCObjectVisitor> fn) noexcept -> void
        {
            self.visit_gco_children(reason, fn);
        }

    } /*namespace process*/
} /*namespace xo*/

/* end IGCObject_DObjectEvent.cpp */
