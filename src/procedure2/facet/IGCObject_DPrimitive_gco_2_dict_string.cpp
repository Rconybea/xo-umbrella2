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
        IGCObject_DPrimitive_gco_2_dict_string::shallow_size(const DPrimitive_gco_2_dict_string & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DPrimitive_gco_2_dict_string::shallow_move(DPrimitive_gco_2_dict_string & self, obj<AAllocator> mm) noexcept -> Opaque
        {
            return self.shallow_move(mm);
        }
        auto
        IGCObject_DPrimitive_gco_2_dict_string::forward_children(DPrimitive_gco_2_dict_string & self, obj<ACollector> gc) noexcept -> size_type
        {
            return self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DPrimitive_gco_2_dict_string.cpp */
