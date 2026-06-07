/** @file IGCObject_DPrimitive_gco_2_dict_string.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DPrimitive_gco_2_dict_string.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DPrimitive_gco_2_dict_string.json5]
**/

#include "detail/IGCObject_DPrimitive_gco_2_dict_string.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DPrimitive_gco_2_dict_string::gco_shallow_move(DPrimitive_gco_2_dict_string & self, obj<AGCObjectVisitor> gc) noexcept -> Opaque
        {
            return self.gco_shallow_move(gc);
        }
        auto
        IGCObject_DPrimitive_gco_2_dict_string::visit_gco_children(DPrimitive_gco_2_dict_string & self, VisitReason reason, obj<AGCObjectVisitor> fn) noexcept -> void
        {
            self.visit_gco_children(reason, fn);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DPrimitive_gco_2_dict_string.cpp */
