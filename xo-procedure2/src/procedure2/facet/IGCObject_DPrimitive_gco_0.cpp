/** @file IGCObject_DPrimitive_gco_0.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DPrimitive_gco_0.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DPrimitive_gco_0.json5]
**/

#include "detail/IGCObject_DPrimitive_gco_0.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DPrimitive_gco_0::shallow_size(const DPrimitive_gco_0 & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DPrimitive_gco_0::shallow_move(DPrimitive_gco_0 & self, obj<ACollector> gc) noexcept -> Opaque
        {
            return self.shallow_move(gc);
        }
        auto
        IGCObject_DPrimitive_gco_0::forward_children(DPrimitive_gco_0 & self, obj<ACollector> gc) noexcept -> void
        {
            self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DPrimitive_gco_0.cpp */
