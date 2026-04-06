/** @file IGCObject_DString.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DString.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DString.json5]
**/

#include "string/IGCObject_DString.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DString::shallow_move(DString & self, obj<ACollector> gc) noexcept -> Opaque
        {
            return self.shallow_move(gc);
        }
        auto
        IGCObject_DString::visit_gco_children(DString & self, obj<AGCObjectVisitor> fn) noexcept -> void
        {
            self.visit_gco_children(fn);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DString.cpp */
