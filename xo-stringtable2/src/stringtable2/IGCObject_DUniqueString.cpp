/** @file IGCObject_DUniqueString.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DUniqueString.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DUniqueString.json5]
**/

#include "uniquestring/IGCObject_DUniqueString.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DUniqueString::gco_shallow_move(DUniqueString & self, obj<AGCObjectVisitor> gc) noexcept -> Opaque
        {
            return self.gco_shallow_move(gc);
        }
        auto
        IGCObject_DUniqueString::visit_gco_children(DUniqueString & self, VisitReason reason, obj<AGCObjectVisitor> fn) noexcept -> void
        {
            self.visit_gco_children(reason, fn);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DUniqueString.cpp */
