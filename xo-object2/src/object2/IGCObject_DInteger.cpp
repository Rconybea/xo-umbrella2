/** @file IGCObject_DInteger.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DInteger.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DInteger.json5]
**/

#include "number/IGCObject_DInteger.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DInteger::shallow_move(DInteger & self, obj<ACollector> gc) noexcept -> Opaque
        {
            return self.shallow_move(gc);
        }
        auto
        IGCObject_DInteger::visit_gco_children(DInteger & self, obj<AGCObjectVisitor> fn) noexcept -> void
        {
            self.visit_gco_children(fn);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DInteger.cpp */
