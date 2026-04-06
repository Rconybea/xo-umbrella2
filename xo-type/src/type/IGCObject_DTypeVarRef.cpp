/** @file IGCObject_DTypeVarRef.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DTypeVarRef.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DTypeVarRef.json5]
**/

#include "typevar/IGCObject_DTypeVarRef.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DTypeVarRef::shallow_move(DTypeVarRef & self, obj<ACollector> gc) noexcept -> Opaque
        {
            return self.shallow_move(gc);
        }
        auto
        IGCObject_DTypeVarRef::visit_gco_children(DTypeVarRef & self, obj<AGCObjectVisitor> fn) noexcept -> void
        {
            self.visit_gco_children(fn);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DTypeVarRef.cpp */
