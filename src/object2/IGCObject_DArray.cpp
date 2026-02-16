/** @file IGCObject_DArray.cpp
 *
 *  Generated automagically from ingredients:
 *  1. code generator:
 *       [xo-facet/codegen/genfacet]
 *     arguments:
 *       --input [idl/IGCObject_DArray.json5]
 *  2. jinja2 template for abstract facet .hpp file:
 *       [iface_facet_any.hpp.j2]
 *  3. idl for facet methods
 *       [idl/IGCObject_DArray.json5]
**/

#include "array/IGCObject_DArray.hpp"

namespace xo {
    namespace scm {
        auto
        IGCObject_DArray::shallow_size(const DArray & self) noexcept -> size_type
        {
            return self.shallow_size();
        }

        auto
        IGCObject_DArray::shallow_copy(const DArray & self, obj<AAllocator> mm) noexcept -> Opaque
        {
            return self.shallow_copy(mm);
        }

        auto
        IGCObject_DArray::forward_children(DArray & self, obj<ACollector> gc) noexcept -> size_type
        {
            return self.forward_children(gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end IGCObject_DArray.cpp */
