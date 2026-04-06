/** @file IGCObject_DArrayType.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DArrayType.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DArrayType.json5]
**/

#include "array/IGCObject_DArrayType.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DArrayType::shallow_move(DArrayType & self, obj<ACollector> gc) noexcept -> Opaque
        {
            return self.shallow_move(gc);
        }
        auto
        IGCObject_DArrayType::visit_gco_children(DArrayType & self, obj<AGCObjectVisitor> fn) noexcept -> void
        {
            self.visit_gco_children(fn);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DArrayType.cpp */
