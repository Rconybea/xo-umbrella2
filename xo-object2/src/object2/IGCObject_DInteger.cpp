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
        IGCObject_DInteger::shallow_size(const DInteger & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DInteger::shallow_copy(const DInteger & self, obj<AAllocator> mm) noexcept -> Opaque
        {
            return self.shallow_copy(mm);
        }

        auto
        IGCObject_DInteger::forward_children(DInteger & self, obj<ACollector> gc) noexcept -> size_type
        {
            return self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DInteger.cpp */
