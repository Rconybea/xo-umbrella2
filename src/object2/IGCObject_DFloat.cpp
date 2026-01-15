/** @file IGCObject_DFloat.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [/Users/roland/proj/xo-umbrella2/xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DFloat.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DFloat.json5]
**/

#include "number/IGCObject_DFloat.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DFloat::shallow_size(const DFloat & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DFloat::shallow_copy(const DFloat & self, obj<AAllocator> mm) noexcept -> Opaque
        {
            return self.shallow_copy(mm);
        }

        auto
        IGCObject_DFloat::forward_children(DFloat & self, obj<ACollector> gc) noexcept -> size_type
        {
            return self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DFloat.cpp */